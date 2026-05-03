/*
 * mpu6050.c
 *
 *  Created on: Feb 21, 2026
 *      Author: Rubin Khadka
 */

#include "mpu6050.h"
#include "i2c1.h"
#include "usart1.h"
#include "timer2.h"

// Global variables for raw and scaled data
volatile MPU6050_Data_t mpu6050_data = { 0 };

// Read a single register from MPU6050
static uint8_t MPU6050_ReadReg(uint8_t reg, uint8_t *data)
{
  I2C1_Start();
  if(I2C1_SendAddr(MPU6050_ADDR, I2C_WRITE) != I2C_OK)
  {
    I2C1_Stop();
    return I2C_ERROR;
  }

  if(I2C1_WriteByte(reg) != I2C_OK)
  {
    I2C1_Stop();
    return I2C_ERROR;
  }

  I2C1_Start();  // Repeated start
  if(I2C1_SendAddr(MPU6050_ADDR, I2C_READ) != I2C_OK)
  {
    I2C1_Stop();
    return I2C_ERROR;
  }

  *data = I2C1_ReadByte(0);  // NACK on last byte
  I2C1_Stop();

  return I2C_OK;
}

// Write a single register to MPU6050
static uint8_t MPU6050_WriteReg(uint8_t reg, uint8_t data)
{
  I2C1_Start();
  if(I2C1_SendAddr(MPU6050_ADDR, I2C_WRITE) != I2C_OK)
  {
    I2C1_Stop();
    return I2C_ERROR;
  }

  if(I2C1_WriteByte(reg) != I2C_OK)
  {
    I2C1_Stop();
    return I2C_ERROR;
  }

  if(I2C1_WriteByte(data) != I2C_OK)
  {
    I2C1_Stop();
    return I2C_ERROR;
  }

  I2C1_Stop();
  return I2C_OK;
}

// Initialize MPU6050
uint8_t MPU6050_Init(void)
{
  uint8_t who_am_i;

  // Check if device is present
  if(MPU6050_ReadReg(MPU6050_WHO_AM_I, &who_am_i) != I2C_OK)
  {
    USART1_SendString("Failed to read WHO_AM_I\r\n");
    return I2C_ERROR;
  }

  if(who_am_i != 0x68)
  {
    USART1_SendString("Wrong device ID!\r\n");
    return I2C_ERROR;
  }

  // Wake up MPU6050 (clear sleep bit)
  if(MPU6050_WriteReg(MPU6050_PWR_MGMT_1, 0x00) != I2C_OK)
  {
    USART1_SendString("Failed to wake device\r\n");
    return I2C_ERROR;
  }

  USART1_SendString("MPU6050 initialized successfully!\r\n");

  TIMER2_Delay_ms(10);

  return I2C_OK;
}

// Read multiple bytes from MPU6050 (burst read)
static uint8_t MPU6050_ReadBurst(uint8_t start_reg, uint8_t *data, uint8_t len)
{
  I2C1_Start();
  if(I2C1_SendAddr(MPU6050_ADDR, I2C_WRITE) != I2C_OK)
  {
    I2C1_Stop();
    return I2C_ERROR;
  }

  if(I2C1_WriteByte(start_reg) != I2C_OK)
  {
    I2C1_Stop();
    return I2C_ERROR;
  }

  I2C1_Start();  // Repeated start
  if(I2C1_SendAddr(MPU6050_ADDR, I2C_READ) != I2C_OK)
  {
    I2C1_Stop();
    return I2C_ERROR;
  }

  for(uint8_t i = 0; i < len; i++)
  {
    // Send ACK for all bytes except the last one
    uint8_t ack = (i < (len - 1)) ? 1 : 0;
    data[i] = I2C1_ReadByte(ack);
  }

  I2C1_Stop();
  return I2C_OK;
}

// Read all sensor data (accelerometer, temperature, gyroscope)
uint8_t MPU6050_ReadAll(void)
{
  uint8_t buffer[14];  // 7 measurements × 2 bytes each

  // Read all 14 bytes starting from ACCEL_XOUT_H (0x3B)
  if(MPU6050_ReadBurst(MPU6050_ACCEL_XOUT_H, buffer, 14) != I2C_OK)
  {
    return I2C_ERROR;
  }

  // Combine high and low bytes for each measurement and put in struct
  mpu6050_data.accel_x = (int16_t) ((buffer[0] << 8) | buffer[1]);
  mpu6050_data.accel_y = (int16_t) ((buffer[2] << 8) | buffer[3]);
  mpu6050_data.accel_z = (int16_t) ((buffer[4] << 8) | buffer[5]);
  mpu6050_data.temp = (int16_t) ((buffer[6] << 8) | buffer[7]);
  mpu6050_data.gyro_x = (int16_t) ((buffer[8] << 8) | buffer[9]);
  mpu6050_data.gyro_y = (int16_t) ((buffer[10] << 8) | buffer[11]);
  mpu6050_data.gyro_z = (int16_t) ((buffer[12] << 8) | buffer[13]);

  return I2C_OK;
}

// Read only accelerometer data
uint8_t MPU6050_ReadAccel(void)
{
  uint8_t buffer[6];

  if(MPU6050_ReadBurst(MPU6050_ACCEL_XOUT_H, buffer, 6) != I2C_OK)
  {
    return I2C_ERROR;
  }

  mpu6050_data.accel_x = (int16_t) ((buffer[0] << 8) | buffer[1]);
  mpu6050_data.accel_y = (int16_t) ((buffer[2] << 8) | buffer[3]);
  mpu6050_data.accel_z = (int16_t) ((buffer[4] << 8) | buffer[5]);

  return I2C_OK;
}

// Read only gyroscope data
uint8_t MPU6050_ReadGyro(void)
{
  uint8_t buffer[6];

  if(MPU6050_ReadBurst(MPU6050_GYRO_XOUT_H, buffer, 6) != I2C_OK)
  {
    return I2C_ERROR;
  }

  mpu6050_data.gyro_x = (int16_t) ((buffer[0] << 8) | buffer[1]);
  mpu6050_data.gyro_y = (int16_t) ((buffer[2] << 8) | buffer[3]);
  mpu6050_data.gyro_z = (int16_t) ((buffer[4] << 8) | buffer[5]);

  return I2C_OK;
}

// Read temperature only
uint8_t MPU6050_ReadTemp(void)
{
  uint8_t buffer[2];

  if(MPU6050_ReadBurst(MPU6050_TEMP_OUT_H, buffer, 2) != I2C_OK)
  {
    return I2C_ERROR;
  }

  mpu6050_data.temp = (int16_t) ((buffer[0] << 8) | buffer[1]);

  return I2C_OK;
}

/*
 * tasks.c
 *
 *  Created on: Mar 8, 2026
 *      Author: Rubin Khadka
 */

#include "stm32f103xb.h"
#include "dht11.h"
#include "dwt.h"
#include "mpu6050.h"
#include "ds3231.h"
#include "timer2.h"
#include "i2c1.h"
#include "can.h"

#define MAX_RETRIES 5

// Global variables to store DHT11 data
volatile uint8_t dht11_humidity1 = 0;
volatile uint8_t dht11_humidity2 = 0;
volatile uint8_t dht11_temperature1 = 0;
volatile uint8_t dht11_temperature2 = 0;

void Task_DHT11_Read(void)
{
  uint8_t hum1, hum2, temp1, temp2, checksum;

  // Disable interrupts for critical section
  uint32_t primask = __get_PRIMASK();
  __disable_irq();

  // Try up to MAX_RETRIES times
  for(int retry = 0; retry < MAX_RETRIES; retry++)
  {
    DHT11_Start();

    if(DHT11_Check_Response())
    {
      hum1 = DHT11_Read();
      hum2 = DHT11_Read();
      temp1 = DHT11_Read();
      temp2 = DHT11_Read();
      checksum = DHT11_Read();

      uint8_t calc = hum1 + hum2 + temp1 + temp2;

      if(calc == checksum)
      {
        dht11_humidity1 = hum1;
        dht11_humidity2 = hum2;
        dht11_temperature1 = temp1;
        dht11_temperature2 = temp2;
      }
    }
  }

  // Re-enable interrupts
  __set_PRIMASK(primask);
}

void Task_MPU6050_Send(void)
{
  // Read MPU6050
  if(MPU6050_ReadAll() == I2C_OK)
  {
    // Send accelerometer raw data
    CAN_SendAccelerometer(mpu6050_data.accel_x, mpu6050_data.accel_y, mpu6050_data.accel_z);

    // Send gyroscope raw data
    CAN_SendGyroscope(mpu6050_data.gyro_x, mpu6050_data.gyro_y, mpu6050_data.gyro_z);
  }
}

/*
 * can.h
 *
 *  Created on: May 1, 2026
 *      Author: Rubin Khadka
 */

#ifndef INC_CAN_H_
#define INC_CAN_H_

#include "main.h"

typedef enum
{
  CAN_ID_POTENTIOMETER  = 0x100,
  CAN_ID_TEMP_HUMD      = 0x101,
  CAN_ID_ACCEL          = 0x102,
  CAN_ID_GYRO           = 0x103,
  CAN_ID_STATUS         = 0x104,
  CAN_ID_TIMESTAMP      = 0x105
} CAN_MessageID_t;

// CAN status
typedef enum
{
  CAN_OK        = 0,
  CAN_ERROR     = 1,
  CAN_BUSY      = 2,
  CAN_TIMEOUT   = 3
} CAN_Status_t;

// Sensor data structure
typedef struct
{
  // ADC Potentiometer
  uint16_t potentiometer;

  // DHT11
  uint8_t temperature;
  uint8_t humidity;

  // MPU6050
  int16_t accel_x;
  int16_t accel_y;
  int16_t accel_z;
  int16_t gyro_x;
  int16_t gyro_y;
  int16_t gyro_z;

  // Status
  uint8_t node_status;
  uint32_t timestamp;
  uint32_t can_sent_count;
  uint32_t can_error_count;
} SensorData_t;

// Global sensor data instance
extern volatile SensorData_t sensor_data;

// Function prototypes
CAN_Status_t CAN_Init(CAN_HandleTypeDef *hcan);
CAN_Status_t CAN_SendPotentiometer(uint16_t value);
CAN_Status_t CAN_SendTempHumidity(uint8_t hum_int, uint8_t hum_dec, uint8_t temp_int, uint8_t temp_dec);
CAN_Status_t CAN_SendAccelerometer(int16_t ax, int16_t ay, int16_t az);
CAN_Status_t CAN_SendGyroscope(int16_t gx, int16_t gy, int16_t gz);
CAN_Status_t CAN_SendStatus(uint8_t status);
CAN_Status_t CAN_SendTimestamp(uint32_t timestamp);
void CAN_SendAllSensorData(void);
uint32_t CAN_GetSentCount(void);
uint32_t CAN_GetErrorCount(void);

#endif /* INC_CAN_H_ */

/*
 * can_app.c
 *
 *  Created on: May 1, 2026
 *      Author: rubin
 */

#include "can.h"
#include "usart1.h"

// CAN handle pointer
static CAN_HandleTypeDef *can_handle;

// Global sensor data
volatile SensorData_t sensor_data = { 0 };

// Internal helper function
static CAN_Status_t CAN_SendMessage(uint16_t id, uint8_t *data, uint8_t len);

/* Initialize CAN */
CAN_Status_t CAN_Init(CAN_HandleTypeDef *hcan)
{
  if(hcan == NULL)
  {
    return CAN_ERROR;
  }

  can_handle = hcan;

  // Reset sensor data
  memset((void*) &sensor_data, 0, sizeof(SensorData_t));

  // Start CAN peripheral
  if(HAL_CAN_Start(can_handle) != HAL_OK)
  {
    USART1_SendString("CAN: Init failed!\r\n");
    return CAN_ERROR;
  }

  USART1_SendString("CAN: Initialized OK\r\n");
  return CAN_OK;
}

/* Generic CAN message sender */
static CAN_Status_t CAN_SendMessage(uint16_t id, uint8_t *data, uint8_t len)
{
  CAN_TxHeaderTypeDef tx_header;
  uint32_t tx_mailbox;

  if(can_handle == NULL)
  {
    return CAN_ERROR;
  }

  if(len > 8)
  {
    return CAN_ERROR;  // CAN max 8 bytes
  }

  // Prepare header
  tx_header.StdId = id;
  tx_header.ExtId = 0;
  tx_header.IDE = CAN_ID_STD;
  tx_header.RTR = CAN_RTR_DATA;
  tx_header.DLC = len;
  tx_header.TransmitGlobalTime = DISABLE;

  // Send message with timeout
  if(HAL_CAN_AddTxMessage(can_handle, &tx_header, data, &tx_mailbox) != HAL_OK)
  {
    sensor_data.can_error_count++;
    return CAN_ERROR;
  }

  sensor_data.can_sent_count++;
  return CAN_OK;
}

/* Send potentiometer value (2 bytes) */
CAN_Status_t CAN_SendPotentiometer(uint16_t value)
{
  uint8_t data[2];

  // Store in global data
  sensor_data.potentiometer = value;

  // Prepare CAN data (Little Endian)
  data[0] = value & 0xFF;        // Low byte
  data[1] = (value >> 8) & 0xFF; // High byte

  return CAN_SendMessage(CAN_ID_POTENTIOMETER, data, 2);
}

/* Send temperature and humidity (2 bytes) */
CAN_Status_t CAN_SendTempHumidity(uint8_t temp, uint8_t humidity)
{
  uint8_t data[2];

  // Store in global data
  sensor_data.temperature = temp;
  sensor_data.humidity = humidity;

  // Prepare CAN data
  data[0] = temp;      // Temperature in Celsius
  data[1] = humidity;  // Relative humidity in %

  return CAN_SendMessage(CAN_ID_TEMP_HUMIDITY, data, 2);
}

/* Send accelerometer data (6 bytes) */
CAN_Status_t CAN_SendAccelerometer(int16_t ax, int16_t ay, int16_t az)
{
  uint8_t data[6];

  // Store in global data
  sensor_data.accel_x = ax;
  sensor_data.accel_y = ay;
  sensor_data.accel_z = az;

  // Prepare CAN data (Big Endian for easier reading)
  data[0] = (ax >> 8) & 0xFF;    // Accel X High
  data[1] = ax & 0xFF;           // Accel X Low
  data[2] = (ay >> 8) & 0xFF;    // Accel Y High
  data[3] = ay & 0xFF;           // Accel Y Low
  data[4] = (az >> 8) & 0xFF;    // Accel Z High
  data[5] = az & 0xFF;           // Accel Z Low

  return CAN_SendMessage(CAN_ID_ACCELEROMETER, data, 6);
}

/* Send gyroscope data (6 bytes) */
CAN_Status_t CAN_SendGyroscope(int16_t gx, int16_t gy, int16_t gz)
{
  uint8_t data[6];

  // Store in global data
  sensor_data.gyro_x = gx;
  sensor_data.gyro_y = gy;
  sensor_data.gyro_z = gz;

  // Prepare CAN data (Big Endian)
  data[0] = (gx >> 8) & 0xFF;    // Gyro X High
  data[1] = gx & 0xFF;           // Gyro X Low
  data[2] = (gy >> 8) & 0xFF;    // Gyro Y High
  data[3] = gy & 0xFF;           // Gyro Y Low
  data[4] = (gz >> 8) & 0xFF;    // Gyro Z High
  data[5] = gz & 0xFF;           // Gyro Z Low

  return CAN_SendMessage(CAN_ID_GYROSCOPE, data, 6);
}

/* Send node status (1 byte) */
CAN_Status_t CAN_SendStatus(uint8_t status)
{
  uint8_t data[1];

  // Store status
  sensor_data.node_status = status;

  // Prepare CAN data
  data[0] = status;  // Status byte (0=OK, 1=Warning, 2=Error, etc.)

  return CAN_SendMessage(CAN_ID_STATUS, data, 1);
}

/* Send timestamp (4 bytes) */
CAN_Status_t CAN_SendTimestamp(uint32_t timestamp)
{
  uint8_t data[4];

  // Store timestamp
  sensor_data.timestamp = timestamp;

  // Prepare CAN data (Big Endian)
  data[0] = (timestamp >> 24) & 0xFF;
  data[1] = (timestamp >> 16) & 0xFF;
  data[2] = (timestamp >> 8) & 0xFF;
  data[3] = timestamp & 0xFF;

  return CAN_SendMessage(CAN_ID_TIMESTAMP, data, 4);
}

/* Get total CAN messages sent */
uint32_t CAN_GetSentCount(void)
{
  return sensor_data.can_sent_count;
}

/* Get total CAN errors */
uint32_t CAN_GetErrorCount(void)
{
  return sensor_data.can_error_count;
}

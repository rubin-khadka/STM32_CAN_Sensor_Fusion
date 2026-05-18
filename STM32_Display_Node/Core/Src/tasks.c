/*
 * tasks.c
 *
 *  Created on: May 2, 2026
 *      Author: Rubin Khadka
 */

#include "lcd.h"
#include "can.h"
#include "usart1.h"

extern volatile struct
{
  uint8_t pot_updated;
  uint8_t temp_hum_updated;
  uint8_t accel_updated;
  uint8_t gyro_updated;
  uint8_t time_updated;
  uint8_t display_mode_updated;

  uint8_t display_mode;
  uint16_t pot_val;
  uint16_t temp_val;
  uint16_t hum_val;
  int16_t accel[3];
  int16_t gyro[3];
  uint8_t hour, min, sec;
} can_rx_data;

// Display modes
typedef enum
{
  DISPLAY_MODE_TEMP_HUM = 0,
  DISPLAY_MODE_DATE_TIME,
  DISPLAY_MODE_ACCEL,
  DISPLAY_MODE_GYRO,
  DISPLAY_MODE_COUNT
} DisplayMode_t;

// Scale accelerometer raw to g
static float Accel_To_g(int16_t raw)
{
  return (float) raw / 16384.0f;
}

// Scale gyroscope raw to °/s
static float Gyro_To_dps(int16_t raw)
{
  return (float) raw / 131.0f;
}

void Task_UpdateDisplay(void)
{
  static DisplayMode_t current_mode = DISPLAY_MODE_TEMP_HUM;

  // Check if new display mode received via CAN
  if(can_rx_data.display_mode_updated)
  {
    can_rx_data.display_mode_updated = 0;

    current_mode = (DisplayMode_t) can_rx_data.display_mode;
    LCD_Clear();
  }

  // Update display based on current mode
  switch(current_mode)
  {
    case DISPLAY_MODE_TEMP_HUM:
      if(can_rx_data.temp_hum_updated)
      {
        can_rx_data.temp_hum_updated = 0;

        // temp_val is stored as (int*10 + dec), e.g., 255 = 25.5°C
        uint8_t temp_int = can_rx_data.temp_val / 10;
        uint8_t temp_dec = can_rx_data.temp_val % 10;

        // hum_val is stored as (int*10 + dec), e.g., 600 = 60.0%
        uint8_t hum_int = can_rx_data.hum_val / 10;
        uint8_t hum_dec = can_rx_data.hum_val % 10;

        LCD_DisplayReading_Temp(temp_int, temp_dec, hum_int, hum_dec);
      }
      break;

    case DISPLAY_MODE_DATE_TIME:
      LCD_SetCursor(0, 0);
      LCD_SendString("TIME:    ");
      LCD_SetCursor(1, 0);
      LCD_SendString("DATE:    ");
      break;

    case DISPLAY_MODE_ACCEL:
      if(can_rx_data.accel_updated)
      {
        can_rx_data.accel_updated = 0;

        // Convert raw values to g
        float ax_g = Accel_To_g(can_rx_data.accel[0]);
        float ay_g = Accel_To_g(can_rx_data.accel[1]);
        float az_g = Accel_To_g(can_rx_data.accel[2]);

        // DEBUG: Print raw and scaled values
        USART1_SendString("\r\n[ACCEL] Raw: X=");
        USART1_SendNumber(can_rx_data.accel[0]);
        USART1_SendString(" Y=");
        USART1_SendNumber(can_rx_data.accel[1]);
        USART1_SendString(" Z=");
        USART1_SendNumber(can_rx_data.accel[2]);

        // Use your new LCD function
        LCD_DisplayAccelScaled(ax_g, ay_g, az_g);
      }
      break;

    case DISPLAY_MODE_GYRO:
      if(can_rx_data.gyro_updated)
      {
        can_rx_data.gyro_updated = 0;

        // Convert raw values to °/s
        float gx_dps = Gyro_To_dps(can_rx_data.gyro[0]);
        float gy_dps = Gyro_To_dps(can_rx_data.gyro[1]);
        float gz_dps = Gyro_To_dps(can_rx_data.gyro[2]);

        // Use your new LCD function
        LCD_DisplayGyroScaled(gx_dps, gy_dps, gz_dps);
      }
      break;

    default:
      break;
  }
}

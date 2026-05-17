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

void Task_UpdateDisplay(void)
{
  static DisplayMode_t current_mode = DISPLAY_MODE_TEMP_HUM;

  // Check if new display mode received via CAN
  if(can_rx_data.display_mode_updated)
  {
    can_rx_data.display_mode_updated = 0;

    current_mode = (DisplayMode_t) can_rx_data.display_mode;
    LCD_Clear();

    USART1_SendString("Display Mode: ");
    USART1_SendNumber(current_mode);
    USART1_SendString("\r\n");
  }

  // Update display based on current mode
  switch(current_mode)
  {
    case DISPLAY_MODE_TEMP_HUM:
      LCD_SetCursor(0, 0);
      LCD_SendString("TEMP:    ");
      LCD_SetCursor(1, 0);
      LCD_SendString("HUMD:    ");
      break;

    case DISPLAY_MODE_DATE_TIME:
      LCD_SetCursor(0, 0);
      LCD_SendString("TIME:    ");
      LCD_SetCursor(1, 0);
      LCD_SendString("DATE:    ");
      break;

    case DISPLAY_MODE_ACCEL:
      LCD_SetCursor(0, 0);
      LCD_SendString("ACCEL:   ");
      LCD_SetCursor(1, 0);
      LCD_SendString("XYZ:     ");
      break;

    case DISPLAY_MODE_GYRO:
      LCD_SetCursor(0, 0);
      LCD_SendString("GYRO:    ");
      LCD_SetCursor(1, 0);
      LCD_SendString("XYZ:     ");
      break;

    default:
      break;
  }
}

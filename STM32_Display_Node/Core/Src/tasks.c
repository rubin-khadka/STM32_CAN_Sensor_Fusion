/*
 * tasks.c
 *
 *  Created on: May 2, 2026
 *      Author: Rubin Khadka
 */

#include "lcd.h"
#include "can.h"

// Holds current value
static uint8_t last_temp_int = 0;
static uint8_t last_temp_dec = 0;
static uint8_t last_hum_int = 0;
static uint8_t last_hum_dec = 0;

void Task_UpdateDisplay(void)
{
  // Extract integer and decimal parts
  uint8_t temp_int = temperature / 10;
  uint8_t temp_dec = temperature % 10;
  uint8_t hum_int = humidity / 10;
  uint8_t hum_dec = humidity % 10;

  // Update only if values changed
  if(temp_int != last_temp_int || temp_dec != last_temp_dec || hum_int != last_hum_int || hum_dec != last_hum_dec)
  {
    last_temp_int = temp_int;
    last_temp_dec = temp_dec;
    last_hum_int = hum_int;
    last_hum_dec = hum_dec;

    LCD_DisplayReading_Temp(temp_int, temp_dec, hum_int, hum_dec);
  }
}

void Task_UpdateDisplay(void)
{
  static DisplayMode_t current_mode = DISPLAY_MODE_TEMP_HUM;

  // Check if new display mode received via CAN
  if(can_rx_data.display_mode_updated)
  {
    can_rx_data.display_mode_updated = 0;

    current_mode = (DisplayMode_t) can_rx_data.display_mode;
    LCD_Clear();  // Clear screen for new mode

    USART1_SendString("Display Mode Changed: ");
    USART1_SendNumber(current_mode);
    USART1_SendString("\r\n");
  }

  // Update display based on current mode
  switch(current_mode)
  {
    case DISPLAY_MODE_TEMP_HUM:
      // Your existing temp/hum display code
      break;

    case DISPLAY_MODE_DATE_TIME:
      // Add time/date display code
      break;

    case DISPLAY_MODE_ACCEL:
      // Add accelerometer display code
      break;

    case DISPLAY_MODE_GYRO:
      // Add gyroscope display code
      break;
  }
}

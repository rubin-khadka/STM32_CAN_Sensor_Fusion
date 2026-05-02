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

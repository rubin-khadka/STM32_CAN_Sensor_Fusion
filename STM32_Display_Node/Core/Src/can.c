/*
 * can.c
 *
 *  Created on: May 1, 2026
 *      Author: Rubin Khadka
 */

#include "can.h"
#include "usart1.h"
#include "main.h"

extern CAN_HandleTypeDef hcan;
extern TIM_HandleTypeDef htim1;

volatile uint16_t led_brightness = 0;

// Structure to pass data from ISR to main loop
volatile struct
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
  uint8_t day, month, year;
} can_rx_data = { 0 };

static uint8_t BCD_To_Dec(uint8_t bcd)
{
  return ((bcd >> 4) * 10) + (bcd & 0x0F);
}

// Print float with 2 decimal places
void Print_Float(float value)
{
  int16_t int_part = (int16_t) value;
  int16_t dec_part = (int16_t) ((value - int_part) * 100);
  if(dec_part < 0)
    dec_part = -dec_part;

  USART1_SendNumber(int_part);
  USART1_SendChar('.');
  if(dec_part < 10)
    USART1_SendChar('0');
  USART1_SendNumber(dec_part);
}

void CAN_Receiver_Init(void)
{
  CAN_FilterTypeDef sFilterConfig;

  sFilterConfig.FilterBank = 0;
  sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
  sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;

  // Accept IDs from 0x100 to 0x107 (mask lower 3 bits)
  sFilterConfig.FilterIdHigh = (0x100 << 5);      // Base ID 0x100
  sFilterConfig.FilterIdLow = 0x0000;
  sFilterConfig.FilterMaskIdHigh = (0x7F8 << 5);  // Mask: 111111111000 (ignore last 3 bits)
  sFilterConfig.FilterMaskIdLow = 0x0000;

  sFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;
  sFilterConfig.FilterActivation = ENABLE;
  sFilterConfig.SlaveStartFilterBank = 14;

  HAL_CAN_ConfigFilter(&hcan, &sFilterConfig);
}

void CAN_Start(void)
{
  HAL_CAN_Start(&hcan);
  HAL_CAN_ActivateNotification(&hcan, CAN_IT_RX_FIFO0_MSG_PENDING);
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
  CAN_RxHeaderTypeDef rxHeader;
  uint8_t rxData[8];

  if(HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rxHeader, rxData) == HAL_OK)
  {
    switch(rxHeader.StdId)
    {
      case CAN_ID_POTENTIOMETER:
        if(rxHeader.DLC == 2)
        {
          can_rx_data.pot_val = (rxData[1] << 8) | rxData[0];
          can_rx_data.pot_updated = 1;
        }
        break;

      case CAN_ID_TEMP_HUMD:
        if(rxHeader.DLC == 4)
        {
          can_rx_data.temp_val = (rxData[1] << 8) | rxData[0];
          can_rx_data.hum_val = (rxData[3] << 8) | rxData[2];
          can_rx_data.temp_hum_updated = 1;
        }
        break;

      case CAN_ID_ACCEL:
        if(rxHeader.DLC == 6)
        {
          can_rx_data.accel[0] = (rxData[0] << 8) | rxData[1];
          can_rx_data.accel[1] = (rxData[2] << 8) | rxData[3];
          can_rx_data.accel[2] = (rxData[4] << 8) | rxData[5];
          can_rx_data.accel_updated = 1;
        }
        break;

      case CAN_ID_GYRO:
        if(rxHeader.DLC == 6)
        {
          can_rx_data.gyro[0] = (rxData[0] << 8) | rxData[1];
          can_rx_data.gyro[1] = (rxData[2] << 8) | rxData[3];
          can_rx_data.gyro[2] = (rxData[4] << 8) | rxData[5];
          can_rx_data.gyro_updated = 1;
        }
        break;

      case CAN_ID_TIMESTAMP:
        if(rxHeader.DLC >= 6)
        {
          can_rx_data.hour = BCD_To_Dec(rxData[0]);
          can_rx_data.min = BCD_To_Dec(rxData[1]);
          can_rx_data.sec = BCD_To_Dec(rxData[2]);
          can_rx_data.day = BCD_To_Dec(rxData[3]);
          can_rx_data.month = BCD_To_Dec(rxData[4]);
          can_rx_data.year = rxData[5];
          can_rx_data.time_updated = 1;
        }
        break;

      case CAN_ID_DISPLAY_MODE:
        if(rxHeader.DLC >= 1)
        {
          can_rx_data.display_mode = rxData[0];
          can_rx_data.display_mode_updated = 1;
        }
        break;

      default:
        break;
    }
  }
}

// Process CAN data in main loop context
void CAN_ProcessNewData(void)
{
  // Process potentiometer for PWM
  if(can_rx_data.pot_updated)
  {
    can_rx_data.pot_updated = 0;

    // Scale 0-4095 to 0-999
    led_brightness = (can_rx_data.pot_val * 999) / 4095;

    // Update PWM
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, led_brightness);
  }
}

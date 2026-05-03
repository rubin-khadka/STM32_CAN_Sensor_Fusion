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

volatile uint16_t temperature = 0;
volatile uint16_t humidity = 0;

static uint8_t last_hour = 0xFF;
static uint8_t last_minute = 0xFF;
static uint8_t last_second = 0xFF;
static uint8_t last_day = 0xFF;
static uint8_t last_month = 0xFF;

static uint8_t BCD_To_Dec(uint8_t bcd)
{
    return ((bcd >> 4) * 10) + (bcd & 0x0F);
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
          uint16_t pot_value = (rxData[1] << 8) | rxData[0];
          USART1_SendString("\r\n[Potentiometer] ");
          USART1_SendNumber(pot_value);

          // Scale 0-4095 (ADC) to 0-999 (PWM ARR)
          led_brightness = (pot_value * 999) / 4095;

          // Update PWM duty cycle
          __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, led_brightness);
        }
        break;

      case CAN_ID_TEMP_HUMD:
        if(rxHeader.DLC == 4)
        {
          temperature = (rxData[1] << 8) | rxData[0];
          humidity = (rxData[3] << 8) | rxData[2];

          USART1_SendString("\r\n[Temp/Hum] ");
          USART1_SendNumber(temperature / 10);
          USART1_SendChar('.');
          USART1_SendNumber(temperature % 10);
          USART1_SendString("C, ");
          USART1_SendNumber(humidity / 10);
          USART1_SendChar('.');
          USART1_SendNumber(humidity % 10);
          USART1_SendString("%");
        }
        break;

      case CAN_ID_TIMESTAMP:
        if(rxHeader.DLC == 5)
        {
          // Convert BCD to decimal
          uint8_t hour = BCD_To_Dec(rxData[0]);
          uint8_t minute = BCD_To_Dec(rxData[1]);
          uint8_t second = BCD_To_Dec(rxData[2]);
          uint8_t day = BCD_To_Dec(rxData[3]);
          uint8_t month = BCD_To_Dec(rxData[4]);

          // Only print if time changed
          if(hour != last_hour || minute != last_minute || second != last_second)
          {
            last_hour = hour;
            last_minute = minute;
            last_second = second;
            last_day = day;
            last_month = month;

            USART1_SendString("\r\nTime: ");
            USART1_SendNumber(hour);
            USART1_SendChar(':');
            if(minute < 10)
              USART1_SendChar('0');
            USART1_SendNumber(minute);
            USART1_SendChar(':');
            if(second < 10)
              USART1_SendChar('0');
            USART1_SendNumber(second);
            USART1_SendString("  Date: ");
            USART1_SendNumber(day);
            USART1_SendChar('/');
            USART1_SendNumber(month);
          }
        }
        break;

      default:
        // Unknown ID - print for debugging
        USART1_SendString("\r\n[Unknown] ID:0x");
        USART1_SendHex(rxHeader.StdId);
        break;
    }
  }
}

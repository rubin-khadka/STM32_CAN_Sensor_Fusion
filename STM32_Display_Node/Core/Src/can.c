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
        }
        break;

      case CAN_ID_TEMP_HUMD:
        if(rxHeader.DLC == 4)
        {
          USART1_SendString("\r\n[Temp/Humidity] ");
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

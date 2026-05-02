/*
 * can.h
 *
 *  Created on: May 1, 2026
 *      Author: Rubin Khadka
 */

#ifndef INC_CAN_H_
#define INC_CAN_H_

#include <stdint.h>

// CAN Message IDs (match with sensor node)
typedef enum {
  CAN_ID_POTENTIOMETER  = 0x100,
  CAN_ID_TEMP_HUMD      = 0x101,
  CAN_ID_ACCEL          = 0x102,
  CAN_ID_GYRO           = 0x103,
  CAN_ID_STATUS         = 0x104,
  CAN_ID_TIMESTAMP      = 0x105
} CAN_MessageID_t;

extern volatile uint16_t temperature;
extern volatile uint16_t humidity;

void CAN_Receiver_Init(void);
void CAN_Start(void);

#endif /* INC_CAN_H_ */

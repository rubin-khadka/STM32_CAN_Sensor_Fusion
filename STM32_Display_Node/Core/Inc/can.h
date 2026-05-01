/*
 * can.h
 *
 *  Created on: May 1, 2026
 *      Author: Rubin Khadka
 */

#ifndef INC_CAN_H_
#define INC_CAN_H_

#include <stdint.h>

#define CAN_POTENTIOMETER_ID  0x100

void CAN_Receiver_Init(void);
void CAN_Start(void);

#endif /* INC_CAN_H_ */

/*
 * tasks.h
 *
 *  Created on: Mar 8, 2026
 *      Author: Rubin Khadka
 */

#ifndef INC_TASKS_H_
#define INC_TASKS_H_

extern volatile uint8_t dht11_humidity1;
extern volatile uint8_t dht11_humidity2;
extern volatile uint8_t dht11_temperature1;
extern volatile uint8_t dht11_temperature2;

void Task_DHT11_Read(void);

#endif /* INC_TASKS_H_ */

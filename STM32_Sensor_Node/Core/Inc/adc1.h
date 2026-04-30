/*
 * adc1.h
 *
 *  Created on: Apr 30, 2026
 *      Author: rubin
 */

#ifndef INC_ADC1_H_
#define INC_ADC1_H_

#include "stdint.h"

#define NUM_CHANNELS 3

// Global variables
extern volatile uint16_t adc_buffer[NUM_CHANNELS];
extern volatile uint8_t adc_data_ready;

// Function Prototypes
void ADC1_Init(void);
void ADC1_DMA_Config(void);
void ADC1_StartConversion(void);
void DMA1_Channel1_IRQHandler(void);


#endif /* INC_ADC1_H_ */

/*
 * adc1.h
 *
 *  Created on: Apr 30, 2026
 *      Author: Rubin Khadka
 */

#ifndef INC_ADC1_H_
#define INC_ADC1_H_

#include "stdint.h"
#include "stm32f103xb.h"

// Single channel configuration
#define ADC_CHANNEL      0    // PA0
#define ADC_BUFFER_SIZE  1    // Single sample

// Global variables
extern volatile uint16_t adc_buffer;
extern volatile uint8_t adc_data_ready;

// Function Prototypes
void ADC1_Init(void);
void ADC1_StartConversion(void);
void DMA1_Channel1_IRQHandler(void);


#endif /* INC_ADC1_H_ */

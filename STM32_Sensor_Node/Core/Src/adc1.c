/*
 * adc1.c
 *
 *  Created on: May 1, 2026
 *      Author: rubin
 */

#include "adc1.h"
#include "main.h"

// Global variables
volatile uint16_t adc_buffer;
volatile uint8_t adc_data_ready = 0;

static void ADC1_DMA_Config(void);

/* Initialize ADC1 for temperature measurement */
void ADC1_Init(void)
{
  // Enable GPIOA clock and configure PA0 as analog
  RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
  GPIOA->CRL &= ~(0xF << 0);    // PA0 as analog mode

  // Enable clocks
  RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
  RCC->AHBENR |= RCC_AHBENR_DMA1EN;

  // Power on ADC
  ADC1->CR2 |= ADC_CR2_ADON;
  for(volatile uint32_t i = 0; i < 10000; i++);  // ~100µs stabilization

  // Configure Sampling times , 55.5 sampling cycles
  ADC1->SMPR2 &= ~(0x7 << 0);
  ADC1->SMPR2 |= (0x5 << 0);

  // Enable Scan Mode
  ADC1->CR1 &= ~ADC_CR1_SCAN;

  // Configure Sequence Length
  ADC1->SQR1 &= ~(0xF << 20);

  // Set channel 0 as first conversion
  ADC1->SQR3 = ADC_CHANNEL;     // SQ1 = Channel 0

  // ADC calibration
  ADC1->CR2 |= ADC_CR2_RSTCAL;
  while(ADC1->CR2 & ADC_CR2_RSTCAL);

  ADC1->CR2 |= ADC_CR2_CAL;
  while(ADC1->CR2 & ADC_CR2_CAL);

  // Enable DMA for ADC
  ADC1->CR2 |= ADC_CR2_DMA;
  ADC1->CR2 &= ~ADC_CR2_CONT;   // Single conversion mode
  ADC1->CR2 &= ~ADC_CR2_ALIGN;  // Right alignment

  // Configure software trigger
  ADC1->CR2 &= ~(0x7 << 17);      // Clear EXTSEL bits
  ADC1->CR2 |= (0x7 << 17);       // EXTSEL[2:0] = 111 (Software trigger)
  ADC1->CR2 |= ADC_CR2_EXTTRIG;   // Enable external trigger

  // DMA Config
  ADC1_DMA_Config();

  adc_buffer = 0;
  adc_data_ready = 0;
}

static void ADC1_DMA_Config(void)
{
  // Disable DMA before configuration
  DMA1_Channel1->CCR &= ~DMA_CCR_EN;

  // Configure DMA Channel 1
  DMA1_Channel1->CPAR = (uint32_t) &(ADC1->DR);
  DMA1_Channel1->CMAR = (uint32_t) &adc_buffer;
  DMA1_Channel1->CNDTR = ADC_BUFFER_SIZE;

  // Configure control register
  DMA1_Channel1->CCR = 0; // Reset the register first
  DMA1_Channel1->CCR = (0b01 << 12) |       // Medium priority
      (0b01 << 10) |                        // Memory: 16-bit
      (0b01 << 8) |                         // Peripheral: 16-bit
      (0 << 7) |                            // Memory increment: DISABLED
      (0 << 6) |                            // Peripheral increment: NO
      (0 << 5) |                            // Circular mode: DISABLED
      (0 << 4) |                            // Direction: Peripheral to memory
      (0 << 3) |                            // TEIE = 0: No error interrupt
      (0 << 2) |                            // HTIE = 0: No half transfer interrupt
      (1 << 1) |                            // Transfer Complete Interrupt Enable
      (0 << 0);                             // Don't enable yet

  // Enable DMA interrupt in NVIC
  NVIC_EnableIRQ(DMA1_Channel1_IRQn);
}

void ADC1_StartConversion(void)
{
  // Reset buffer and flag
  adc_buffer = 0;
  adc_data_ready = 0;

  // Disable DMA if still running
  if(DMA1_Channel1->CCR & DMA_CCR_EN)
  {
    DMA1_Channel1->CCR &= ~DMA_CCR_EN;
    DMA1->IFCR |= DMA_IFCR_CTCIF1;  // Clear any pending flags
  }

  // Reload DMA
  DMA1_Channel1->CNDTR = ADC_BUFFER_SIZE;
  DMA1_Channel1->CMAR = (uint32_t) &adc_buffer;

  // Enable DMA
  DMA1_Channel1->CCR |= DMA_CCR_EN;

  // Start ADC conversion
  ADC1->CR2 |= ADC_CR2_SWSTART;
}

void DMA1_Channel1_IRQHandler(void)
{
  if(DMA1->ISR & DMA_ISR_TCIF1)
  {
    // Clear transfer complete flag
    DMA1->IFCR |= DMA_IFCR_CTCIF1;

    // Disable DMA (single conversion mode)
    DMA1_Channel1->CCR &= ~DMA_CCR_EN;

    // Signal data ready
    adc_data_ready = 1;
  }
}


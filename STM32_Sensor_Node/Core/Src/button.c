/*
 * button.c
 *
 *  Created on: May 17, 2026
 *      Author: Rubin Khadka
 */

#include "stm32f103xb.h"
#include "button.h"
#include "can.h"
#include "usart1.h"

// Current display mode
static volatile DisplayMode_t current_mode = DISPLAY_MODE_TEMP_HUM;

// Button states for debouncing
static volatile uint8_t button1_pressed = 0;

volatile uint8_t button_mode_changed = 0;

void Button_Init(void)
{
  // Enable Clocks
  RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_AFIOEN;

  // GPIO Configuration for button
  // PA1 for display mode switch
  GPIOA->CRL &= ~(GPIO_CRL_MODE1 | GPIO_CRL_CNF1);
  GPIOA->CRL |= GPIO_CRL_CNF1_1;  // Input mode pull-up/pull-down
  GPIOA->ODR |= GPIO_ODR_ODR1;    // GPIO pull-up on PA1

  // Connect PA1 to External Interrupt 1
  AFIO->EXTICR[0] &= ~AFIO_EXTICR1_EXTI1;
  AFIO->EXTICR[0] |= AFIO_EXTICR1_EXTI1_PA;

  // Disable interrupt while configuring
  EXTI->IMR &= ~(EXTI_IMR_MR1);

  // Configure trigger edge (Falling edge - button press)
  EXTI->FTSR |= EXTI_FTSR_TR1;
  EXTI->RTSR &= ~(EXTI_RTSR_TR1);

  // Clear any pending interrupt
  EXTI->PR |= EXTI_PR_PR1;

  // Enable interrupt
  EXTI->IMR |= EXTI_IMR_MR1;

  // Enable in NVIC
  NVIC_EnableIRQ(EXTI1_IRQn);
}

// EXTI1 Interrupt Handler
void EXTI1_IRQHandler(void)
{
  if(EXTI->PR & EXTI_PR_PR1)
  {
    EXTI->IMR &= ~EXTI_IMR_MR1;
    EXTI->PR |= EXTI_PR_PR1;

    button1_pressed = 1;

    TIM4->CNT = 0;
    TIM4->SR &= ~TIM_SR_UIF;
    TIM4->CR1 |= TIM_CR1_CEN;
  }
}

void TIMER4_Init(void)
{
  // Enable TIM4 clock
  RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;

  // Small delay for clock to stabilize
  for(volatile int i = 0; i < 10; i++);

  // Configure for 0.1ms resolution at 72MHz
  TIM4->PSC = 7200 - 1;     // Prescaler = 7199
  TIM4->ARR = 500 - 1;      // 500 ticks = 50ms

  // Enable interrupt
  TIM4->DIER |= TIM_DIER_UIE;

  // Clear any pending interrupt flags
  TIM4->SR &= ~TIM_SR_UIF;

  // Button external interrupt will enable the timer
  TIM4->CR1 &= ~TIM_CR1_CEN;

  // Enable TIM4 interrupt in NVIC
  NVIC_EnableIRQ(TIM4_IRQn);
}

void TIM4_IRQHandler(void)
{
  if(TIM4->SR & TIM_SR_UIF)
  {
    TIM4->SR &= ~TIM_SR_UIF;
    TIM4->CR1 &= ~TIM_CR1_CEN;

    // Button 1
    if(button1_pressed)
    {
      if(!(GPIOA->IDR & GPIO_IDR_IDR1))
      {
        button1_pressed = 0;
        Button_NextMode();
        button_mode_changed = 1;
      }
      EXTI->IMR |= EXTI_IMR_MR1;
    }
  }
}

DisplayMode_t Button_GetMode(void)
{
  return current_mode;
}

// Change to next mode
void Button_NextMode(void)
{
  current_mode++;
  if(current_mode >= DISPLAY_MODE_COUNT)
  {
    current_mode = DISPLAY_MODE_TEMP_HUM;
  }
}

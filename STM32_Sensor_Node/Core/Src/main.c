/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2026 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "usart1.h"
#include "adc1.h"
#include "dwt.h"
#include "i2c2.h"
#include "ds3231.h"
#include "dht11.h"
#include "mpu6050.h"
#include "timer2.h"
#include "timer3.h"
#include "can.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
#define ADC_READ_TICKS        10     // Read ADC every 100ms
#define DHT11_READ_TICKS      100    // Read DHT11 every 1 second
#define MPU_READ_TICKS        5      // Read MPU6050 every 50ms
#define STATUS_SEND_TICKS     100    // Send status every 1 second
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
CAN_HandleTypeDef hcan;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_CAN_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_CAN_Init();
  /* USER CODE BEGIN 2 */

  DWT_Init();
  ADC1_Init();
  USART1_Init();
  TIMER2_Init();
  I2C2_Init();

  // Initialize CAN application
  CAN_Init(&hcan);

  // Sensor read counters
  uint16_t adc_count = 0;
  uint16_t dht_count = 0;
  uint16_t mpu_count = 0;
  uint16_t status_count = 0;

  // Initialize sensors
  DS3231_Init();
  MPU6050_Init();
  DHT11_Init();

  USART1_SendString("Sensor Node Ready\r\n");
  USART1_SendString("==================\r\n");

  TIMER3_SetupPeriod(10);  // 10ms period
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while(1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

    // ADC Potentiometer - Read & Send @ 100ms
    if(adc_count++ >= ADC_READ_TICKS)
    {
      adc_count = 0;

      // Start ADC conversion
      ADC1_StartConversion();

      // Wait for conversion complete (with timeout)
      uint32_t timeout = 100000;
      while(!adc_data_ready && --timeout);

      if(adc_data_ready)
      {
        uint16_t pot_value = adc_buffer;
        adc_data_ready = 0;

        // Store in global data structure
        sensor_data.potentiometer = pot_value;

        // Send IMMEDIATELY via CAN
        if(CAN_SendPotentiometer(pot_value) == CAN_OK)
        {
          // Optional: Debug output every 10th reading (1 second)
          if(adc_count % 10 == 0)
          {
            USART1_SendString("ADC: ");
            USART1_SendNumber(pot_value);
            USART1_SendString(" -> CAN OK\r\n");
          }
        }
        else
        {
          USART1_SendString("CAN Error: Potentiometer!\r\n");
        }
      }
      else
      {
        USART1_SendString("ADC Timeout!\r\n");
      }
    }

    // DHT11 Temperature & Humidity - Read & Send @ 1s
    if(dht_count++ >= DHT11_READ_TICKS)
    {
      dht_count = 0;

      uint8_t test_temp = 25;
      uint8_t test_hum = 60;
      CAN_SendTempHumidity(test_temp, test_hum);

      USART1_SendString("DHT11: ");
      USART1_SendNumber(test_temp);
      USART1_SendString("°C, ");
      USART1_SendNumber(test_hum);
      USART1_SendString("% -> CAN OK\r\n");
    }

    // MPU6050 Motion Data - Read & Send @ 50ms
    if(mpu_count++ >= MPU_READ_TICKS)
    {
      mpu_count = 0;

      // Temporary test data
      static int16_t test_val = 0;
      test_val += 100;
      CAN_SendAccelerometer(test_val, test_val * 2, test_val * 3);
      CAN_SendGyroscope(test_val / 10, test_val / 20, test_val / 30);

      // Debug every 20th reading (1 second)
      if(mpu_count % 20 == 0)
      {
        USART1_SendString("MPU6050: Data sent via CAN\r\n");
      }
    }

    // Node Status - Send @ 1 second
    if(status_count++ >= STATUS_SEND_TICKS)
    {
      status_count = 0;

      // Send node status (0 = OK)
      CAN_SendStatus(0x00);

      // Send statistics
      USART1_SendString("Status: OK | CAN Sent: ");
      USART1_SendNumber(CAN_GetSentCount());
      USART1_SendString(" | Errors: ");
      USART1_SendNumber(CAN_GetErrorCount());
      USART1_SendString("\r\n");
    }

    TIMER3_WaitPeriod(); // 10ms heartbeat
  }
  /* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
  RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

  /** Initializes the RCC Oscillators according to the specified parameters
   * in the RCC_OscInitTypeDef structure.
   */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
   */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }

  /** Enables the Clock Security System
   */
  HAL_RCC_EnableCSS();
}

/**
 * @brief CAN Initialization Function
 * @param None
 * @retval None
 */
static void MX_CAN_Init(void)
{

  /* USER CODE BEGIN CAN_Init 0 */

  /* USER CODE END CAN_Init 0 */

  /* USER CODE BEGIN CAN_Init 1 */

  /* USER CODE END CAN_Init 1 */
  hcan.Instance = CAN1;
  hcan.Init.Prescaler = 9;
  hcan.Init.Mode = CAN_MODE_NORMAL;
  hcan.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan.Init.TimeSeg1 = CAN_BS1_6TQ;
  hcan.Init.TimeSeg2 = CAN_BS2_1TQ;
  hcan.Init.TimeTriggeredMode = DISABLE;
  hcan.Init.AutoBusOff = ENABLE;
  hcan.Init.AutoWakeUp = DISABLE;
  hcan.Init.AutoRetransmission = ENABLE;
  hcan.Init.ReceiveFifoLocked = DISABLE;
  hcan.Init.TransmitFifoPriority = DISABLE;
  if(HAL_CAN_Init(&hcan) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN_Init 2 */

  /* USER CODE END CAN_Init 2 */

}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void)
{
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while(1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

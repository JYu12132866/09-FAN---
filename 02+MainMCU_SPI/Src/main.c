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
#include "i2c.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "wwdg.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
static volatile uint32_t wwdg_counter = 0;

/* SPI test related variables */
static uint8_t spi_test_command_received = 0;
static uint8_t spi_test_buffer[32];
static uint8_t spi_rx_buffer[128];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* SPI test function declarations */
void Process_SPI_Test_Command(void);
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
  MX_USART1_UART_Init();
  MX_TIM4_Init();
  MX_SPI2_Init(); // 初始化SPI2
  // MX_USART3_UART_Init(); // 不需要使用USART3


  /* Disable JTAG to release PA15 (JTMS) as GPIO */
  __HAL_RCC_AFIO_CLK_ENABLE();
  __HAL_AFIO_REMAP_SWJ_NOJTAG();

  /* 启用窗口看门狗 */
  MX_WWDG_Init();

  /* 启动TIM4 PWM通道 */
  HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_3);
  HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_4);
  
  /* 初始化LED状态 - 通过PWM控制 */
  __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_3, 500000); // 50%占空比
  __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_4, 0); // 0%占空比

  /* USER CODE BEGIN 2 */
  /* Send welcome message */
  const char *welcome_msg = "\r\nSTM32 SPI Transfer Ready\r\n";
  HAL_UART_Transmit(&huart1, (uint8_t*)welcome_msg, strlen(welcome_msg), HAL_MAX_DELAY);

  /* 配置MCO引脚输出系统时钟（用于示波器测量） */
  /* PA8作为MCO1引脚 */
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  __HAL_RCC_GPIOA_CLK_ENABLE();
  GPIO_InitStruct.Pin = GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  
  /* 配置MCO1输出系统时钟，不分频 */
  HAL_RCC_MCOConfig(RCC_MCO1, RCC_MCO1SOURCE_SYSCLK, RCC_MCODIV_1);

  /* 启动UART中断接收 */
  rx_index = 0;
  HAL_UART_Receive_IT(&huart1, &rx_buffer[rx_index], 1);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    /* Implement 1-second delay while feeding watchdog */
    /* WWDG needs to be fed every ~25μs */
    /* 1 second / 25μs = 40000 feedings */

    uint32_t feed_count = 0;
    const uint32_t FEED_TARGET = 40000;

    while (feed_count < FEED_TARGET) {
      /* Short delay (about 25μs) */
      volatile uint32_t delay = 500;
      while (delay--);
      
      /* Feed watchdog */
      HAL_WWDG_Refresh(&hwwdg);
      feed_count++;
    }

    /* Toggle LED state - via PWM control */
    static uint32_t led3_compare = 500000;
    static uint32_t led4_compare = 0;
    
    if (led3_compare == 500000) {
      led3_compare = 0;
      led4_compare = 500000;
    } else {
      led3_compare = 500000;
      led4_compare = 0;
    }
    
    __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_3, led3_compare);
    __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_4, led4_compare);

    /* Check UART reception - process received string */
    uint8_t rx_data[50];
    uint8_t len = USART1_GetReceivedData(rx_data, sizeof(rx_data));
    if (len > 0) {
      // `USART1_GetReceivedData` 已保证不包含 \r/\n，这里只做长度裁剪并直接送 SPI
      // 需要额外拼接 "\r\n"（2字节）以及结尾 '\0'（1字节）
      if (len > sizeof(spi_test_buffer) - 3) {
        len = sizeof(spi_test_buffer) - 3;
      }
      memcpy(spi_test_buffer, rx_data, len);
      spi_test_buffer[len] = '\0';
      spi_test_command_received = 1;

      // 回显整行（避免在中断里阻塞发送导致丢字节）
      HAL_UART_Transmit(&huart1, spi_test_buffer, len, 100);
      HAL_UART_Transmit(&huart1, (uint8_t*)"\r\n", 2, 100);
    }
    
    /* Process SPI command */
    if (spi_test_command_received) {
      Process_SPI_Test_Command();
      spi_test_command_received = 0;
    }
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.HSEState = RCC_HSE_OFF;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL16;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */


/**
  * @brief  Process SPI test command
  * @retval None
  */
void Process_SPI_Test_Command(void)
{
    uint16_t payload_len = strlen((char*)spi_test_buffer);

    if (payload_len == 0) {
        HAL_UART_Transmit(&huart1, (uint8_t*)"SPI Error: Empty command\r\n", 26, 100);
        return;
    }

    if (payload_len > 60) {
        HAL_UART_Transmit(&huart1, (uint8_t*)"SPI Error: Command too long\r\n", 29, 100);
        return;
    }

    // 构造发送帧：payload + \r\n
    uint8_t tx_frame[64];
    memset(tx_frame, 0, sizeof(tx_frame));
    memcpy(tx_frame, spi_test_buffer, payload_len);
    tx_frame[payload_len] = '\r';
    tx_frame[payload_len + 1] = '\n';
    uint16_t tx_len = payload_len + 2;

    HAL_UART_Transmit(&huart1, (uint8_t*)"SPI TX: ", 8, 100);
    HAL_UART_Transmit(&huart1, tx_frame, tx_len, 100);
    HAL_UART_Transmit(&huart1, (uint8_t*)"\r\n", 2, 100);

    // 接收缓冲区
    uint8_t rx_frame[128];
    memset(rx_frame, 0, sizeof(rx_frame));

    // 使用 SPI2_TransmitThenReceive：在同一次片选内先发送命令，再打时钟接收响应
    // 从机处理后会返回响应，接收长度使用最大128字节
    HAL_StatusTypeDef status = SPI2_TransmitThenReceive(tx_frame, tx_len, rx_frame, 128);

    if (status == HAL_OK) {
        // 查找有效响应数据（跳过前导0x00或0xFF）
        uint16_t start_idx = 0;
        while (start_idx < 128 && (rx_frame[start_idx] == 0x00 || rx_frame[start_idx] == 0xFF)) {
            start_idx++;
        }
        
        // 找到响应的结束位置（查找第一个0字节或不可打印字符）
        uint16_t resp_len = 0;
        for (uint16_t i = start_idx; i < 128; i++) {
            if (rx_frame[i] == 0 || rx_frame[i] < 0x20 || rx_frame[i] == 0xFF) {
                break;
            }
            resp_len++;
        }
        
        HAL_UART_Transmit(&huart1, (uint8_t*)"SPI RX: ", 8, 100);
        if (resp_len > 0) {
            HAL_UART_Transmit(&huart1, &rx_frame[start_idx], resp_len, 100);
        } else {
            HAL_UART_Transmit(&huart1, (uint8_t*)"(empty)", 7, 100);
        }
        HAL_UART_Transmit(&huart1, (uint8_t*)"\r\n", 2, 100);
    } else {
        HAL_UART_Transmit(&huart1, (uint8_t*)"SPI Error: Communication failed\r\n", 33, 100);
    }
}

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
  while (1)
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

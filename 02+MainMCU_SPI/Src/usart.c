/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    usart.c
  * @brief   This file provides code for the configuration
  *          of the USART instances.
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
#include "usart.h"
#include <string.h>

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

UART_HandleTypeDef huart1;

/* USART1 init function */

void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(uartHandle->Instance==USART1)
  {
  /* USER CODE BEGIN USART1_MspInit 0 */

  /* USER CODE END USART1_MspInit 0 */
    /* USART1 clock enable */
    __HAL_RCC_USART1_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**USART1 GPIO Configuration
    PA9     ------> USART1_TX
    PA10     ------> USART1_RX
    */
    GPIO_InitStruct.Pin = UART1_TX_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(UART1_TX_GPIO_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = UART1_RX_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(UART1_RX_GPIO_Port, &GPIO_InitStruct);

    /* USART1 interrupt Init */
    HAL_NVIC_SetPriority(USART1_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USART1_IRQn);
  /* USER CODE BEGIN USART1_MspInit 1 */

  /* USER CODE END USART1_MspInit 1 */
  }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef* uartHandle)
{

  if(uartHandle->Instance==USART1)
  {
  /* USER CODE BEGIN USART1_MspDeInit 0 */

  /* USER CODE END USART1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART1_CLK_DISABLE();

    /**USART1 GPIO Configuration
    PA9     ------> USART1_TX
    PA10     ------> USART1_RX
    */
    HAL_GPIO_DeInit(UART1_TX_GPIO_Port, UART1_TX_Pin|UART1_RX_Pin);

    /* USART1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USART1_IRQn);
  /* USER CODE BEGIN USART1_MspDeInit 1 */

  /* USER CODE END USART1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

// 串口接收：中断里拼包(一行) + 多帧队列，避免主循环未取走就被覆盖
#define RX_BUFFER_SIZE 100
// 主循环可能有较长阻塞段(喂狗/SPI)，队列适当加深避免短时间内多行输入被丢
#define RX_FRAME_QUEUE_SIZE 16

uint8_t rx_buffer[RX_BUFFER_SIZE];          // 兼容原先 HAL_UART_Receive_IT 的单字节接收落点
volatile uint8_t rx_index = 0;              // 保留对外符号(部分代码/文档可能引用)

static uint8_t build_line[RX_BUFFER_SIZE];
static volatile uint8_t build_len = 0;

static uint8_t frame_q[RX_FRAME_QUEUE_SIZE][RX_BUFFER_SIZE];
static volatile uint8_t frame_len_q[RX_FRAME_QUEUE_SIZE];
static volatile uint8_t q_head = 0; // 写入位置
static volatile uint8_t q_tail = 0; // 读取位置
static volatile uint8_t q_count = 0;

static void UART1_QueuePushLineFromISR(const uint8_t *line, uint8_t len)
{
  if (len >= RX_BUFFER_SIZE) {
    len = RX_BUFFER_SIZE - 1;
  }

  if (q_count >= RX_FRAME_QUEUE_SIZE) {
    // 队列满：丢弃最旧的一帧，保证最新数据尽量不丢
    q_tail = (uint8_t)((q_tail + 1) % RX_FRAME_QUEUE_SIZE);
    q_count--;
  }

  memcpy(frame_q[q_head], line, len);
  frame_q[q_head][len] = '\0';
  frame_len_q[q_head] = len;
  q_head = (uint8_t)((q_head + 1) % RX_FRAME_QUEUE_SIZE);
  q_count++;
}

/**
  * @brief  串口接收完成回调函数
  * @param  huart: UART句柄
  * @retval None
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == USART1)
  {
    // 获取接收到的字符
    uint8_t received_char = rx_buffer[rx_index];

    // 行结束：\r 或 \n 都作为结束符(兼容不同串口助手设置)
    if (received_char == '\r' || received_char == '\n')
    {
      // 入队当前行(不含 \r/\n)
      UART1_QueuePushLineFromISR(build_line, build_len);

      // 重置行构建缓冲，准备下一行
      build_len = 0;
      rx_index = 0;
      HAL_UART_Receive_IT(&huart1, &rx_buffer[rx_index], 1);
    }
    else
    {
      // 支持退格：0x08 或 0x7F
      if (received_char == 0x08 || received_char == 0x7F) {
        if (build_len > 0) {
          build_len--;
        }
        rx_index = 0;
        HAL_UART_Receive_IT(&huart1, &rx_buffer[rx_index], 1);
        return;
      }

      // 累积到当前行缓冲
      if (build_len < (RX_BUFFER_SIZE - 1)) {
        build_line[build_len++] = received_char;
      } else {
        // 行太长：强制结包入队并清空，避免死等
        UART1_QueuePushLineFromISR(build_line, build_len);
        build_len = 0;
      }

      // 继续接收下一个字节
      rx_index = 0;
      HAL_UART_Receive_IT(&huart1, &rx_buffer[rx_index], 1);
    }
  }
}

/**
  * @brief  获取接收到的字符串
  * @param  buffer: 输出缓冲区
  * @param  size: 缓冲区大小
  * @retval 接收的字节数
  */
uint8_t USART1_GetReceivedData(uint8_t *buffer, uint8_t size)
{
  if (buffer == NULL || size == 0) return 0;

  // 临界区：避免与中断同时操作队列指针/计数
  uint32_t primask = __get_PRIMASK();
  __disable_irq();

  if (q_count == 0) {
    if (!primask) __enable_irq();
    buffer[0] = '\0';
    return 0;
  }

  uint8_t len = frame_len_q[q_tail];
  if (len > (uint8_t)(size - 1)) {
    len = (uint8_t)(size - 1);
  }
  memcpy(buffer, frame_q[q_tail], len);
  buffer[len] = '\0';

  q_tail = (uint8_t)((q_tail + 1) % RX_FRAME_QUEUE_SIZE);
  q_count--;

  if (!primask) __enable_irq();
  return len;
}

/* USER CODE END 1 */

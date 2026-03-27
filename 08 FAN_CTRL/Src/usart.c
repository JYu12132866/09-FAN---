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
UART_HandleTypeDef huart2;

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

/* USART2 init function */

void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 9600;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

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
  else if(uartHandle->Instance==USART2)
  {
  /* USER CODE BEGIN USART2_MspInit 0 */

  /* USER CODE END USART2_MspInit 0 */
    /* USART2 clock enable */
    __HAL_RCC_USART2_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**USART2 GPIO Configuration
    PA2     ------> USART2_TX
    PA3     ------> USART2_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_3;
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        /* USART2 interrupt Init */
        HAL_NVIC_SetPriority(USART2_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(USART2_IRQn);
      /* USER CODE BEGIN USART2_MspInit 1 */

      /* USER CODE END USART2_MspInit 1 */
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
  else if(uartHandle->Instance==USART2)
  {
  /* USER CODE BEGIN USART2_MspDeInit 0 */

  /* USER CODE END USART2_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART2_CLK_DISABLE();

    /**USART2 GPIO Configuration
    PA2     ------> USART2_TX
    PA3     ------> USART2_RX
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_2|GPIO_PIN_3);

    /* USART2 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USART2_IRQn);
  /* USER CODE BEGIN USART2_MspDeInit 1 */

  /* USER CODE END USART2_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

// 串口接收缓冲区定义
#define RX_BUFFER_SIZE 100
uint8_t rx_buffer[RX_BUFFER_SIZE];
volatile uint8_t rx_index = 0;
static uint8_t cmd_buffer[RX_BUFFER_SIZE];
static volatile uint8_t cmd_ready = 0;

// USART2接收缓冲区定义
#define RX_BUFFER_SIZE_USART2 100
uint8_t rx_buffer_usart2[RX_BUFFER_SIZE_USART2];
volatile uint8_t rx_index_usart2 = 0;
static uint8_t cmd_buffer_usart2[RX_BUFFER_SIZE_USART2];
volatile uint8_t usart2_cmd_ready = 0;

/* 命令处理回调函数指针 */
static USART_CommandHandler_t usart1_cmd_handler = NULL;
static USART_CommandHandler_t usart2_cmd_handler = NULL;

/**
  * @brief  串口接收完成回调函数
  * @param  huart: UART句柄
  * @retval None
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == USART1)
  {
    /* 兼容 PC 端发送 CRLF：忽略单独的 '\n' 字符 */
    if (rx_buffer[rx_index] == '\n')
    {
      HAL_UART_Receive_IT(&huart1, &rx_buffer[rx_index], 1);
      return;
    }

    // 检查是否收到回车
    if (rx_buffer[rx_index] == '\r')
    {
      // 确保至少有一个有效字符
      if (rx_index > 0)
      {
        /* 将收到的一整条命令复制到独立缓冲，避免被下一条接收覆盖导致“丢首字符/错位” */
        if (!cmd_ready)
        {
          uint8_t copy_len = rx_index;
          if (copy_len >= RX_BUFFER_SIZE) copy_len = RX_BUFFER_SIZE - 1;
          memcpy(cmd_buffer, rx_buffer, copy_len);
          cmd_buffer[copy_len] = '\0';
          cmd_ready = 1;
        }
      }
      // 重置索引，准备下一次接收
      rx_index = 0;
      HAL_UART_Receive_IT(&huart1, &rx_buffer[rx_index], 1);
    }
    else if (rx_index >= RX_BUFFER_SIZE - 1)
    {
      // 缓冲区已满，强制结束接收
      rx_buffer[rx_index] = '\0';
      if (!cmd_ready)
      {
        memcpy(cmd_buffer, rx_buffer, RX_BUFFER_SIZE - 1);
        cmd_buffer[RX_BUFFER_SIZE - 1] = '\0';
        cmd_ready = 1;
      }
      // 重置索引，准备下一次接收
      rx_index = 0;
      HAL_UART_Receive_IT(&huart1, &rx_buffer[rx_index], 1);
    }
    else
    {
      rx_index++;
      // 继续接收下一个字节
      HAL_UART_Receive_IT(&huart1, &rx_buffer[rx_index], 1);
    }
  }
  else if (huart->Instance == USART2)
  {
    // TEC Controller may use different line endings or no line ending
    // Check for CR or LF as line ending
    if (rx_buffer_usart2[rx_index_usart2] == '\r' || rx_buffer_usart2[rx_index_usart2] == '\n')
    {
      // 确保至少有一个有效字符
      if (rx_index_usart2 > 0)
      {
        /* 将收到的一整条命令复制到独立缓冲，避免被下一条接收覆盖导致"丢首字符/错位" */
        if (!usart2_cmd_ready)
        {
          uint8_t copy_len = rx_index_usart2;
          if (copy_len >= RX_BUFFER_SIZE_USART2) copy_len = RX_BUFFER_SIZE_USART2 - 1;
          memcpy(cmd_buffer_usart2, rx_buffer_usart2, copy_len);
          cmd_buffer_usart2[copy_len] = '\0';
          usart2_cmd_ready = 1;
        }
      }
      // 重置索引，准备下一次接收
      rx_index_usart2 = 0;
      HAL_UART_Receive_IT(&huart2, &rx_buffer_usart2[rx_index_usart2], 1);
    }
    else if (rx_index_usart2 >= RX_BUFFER_SIZE_USART2 - 1)
    {
      // 缓冲区已满，强制结束接收
      rx_buffer_usart2[rx_index_usart2] = '\0';
      if (!usart2_cmd_ready)
      {
        memcpy(cmd_buffer_usart2, rx_buffer_usart2, RX_BUFFER_SIZE_USART2 - 1);
        cmd_buffer_usart2[RX_BUFFER_SIZE_USART2 - 1] = '\0';
        usart2_cmd_ready = 1;
      }
      // 重置索引，准备下一次接收
      rx_index_usart2 = 0;
      HAL_UART_Receive_IT(&huart2, &rx_buffer_usart2[rx_index_usart2], 1);
    }
    else
    {
      rx_index_usart2++;
      // 继续接收下一个字节
      HAL_UART_Receive_IT(&huart2, &rx_buffer_usart2[rx_index_usart2], 1);
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
  if (!cmd_ready) return 0;
  
  uint8_t len = 0;
  // 计算实际接收到的字节数（不包括结束符）
  while (len < RX_BUFFER_SIZE && cmd_buffer[len] != '\0') {
    len++;
  }
  
  // 确保至少有一个有效字符
  if (len > 0)
  {
    if (len > size - 1) len = size - 1;
    
    memcpy(buffer, cmd_buffer, len);
    buffer[len] = '\0';
    
    // 重置接收状态
    cmd_ready = 0;
    
    return len;
  }
  
  // 如果没有有效字符，重置接收状态
  cmd_ready = 0;
  return 0;
}

/**
  * @brief  获取USART2接收到的字符串
  * @param  buffer: 输出缓冲区
  * @param  size: 缓冲区大小
  * @retval 接收的字节数
  */
uint8_t USART2_GetReceivedData(uint8_t *buffer, uint8_t size)
{
  if (!usart2_cmd_ready) return 0;
  
  uint8_t len = 0;
  // 计算实际接收到的字节数（不包括结束符）
  while (len < RX_BUFFER_SIZE_USART2 && cmd_buffer_usart2[len] != '\0') {
    len++;
  }
  
  // 确保至少有一个有效字符
  if (len > 0)
  {
    if (len > size - 1) len = size - 1;
    
    memcpy(buffer, cmd_buffer_usart2, len);
    buffer[len] = '\0';
    
    // 重置接收状态
    usart2_cmd_ready = 0;
    
    return len;
  }
  
  // 如果没有有效字符，重置接收状态
  usart2_cmd_ready = 0;
  return 0;
}

/**
  * @brief  注册USART1命令处理函数
  * @param  handler: 命令处理函数指针
  * @retval None
  */
void USART1_RegisterCommandHandler(USART_CommandHandler_t handler)
{
    usart1_cmd_handler = handler;
}

/**
  * @brief  处理USART1命令（在main循环中调用）
  * @retval None
  */
void USART1_Process(void)
{
    uint8_t rx_data[RX_BUFFER_SIZE];
    uint8_t len = USART1_GetReceivedData(rx_data, sizeof(rx_data));
    
    if (len > 0 && usart1_cmd_handler != NULL)
    {
        usart1_cmd_handler(rx_data, len);
    }
}

/**
  * @brief  处理USART2命令（在main循环中调用）
  * @retval None
  */
void USART2_Process(void)
{
    uint8_t rx_data[RX_BUFFER_SIZE_USART2];
    uint8_t len = USART2_GetReceivedData(rx_data, sizeof(rx_data));
    
    if (len > 0 && usart2_cmd_handler != NULL)
    {
        usart2_cmd_handler(rx_data, len);
    }
}

/* USER CODE END 1 */

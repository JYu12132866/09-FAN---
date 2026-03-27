/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    usart.h
  * @brief   This file contains all the function prototypes for
  *          the usart.c file
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USART_H__
#define __USART_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;

/* USER CODE BEGIN Private defines */

#define RX_BUFFER_SIZE 100
extern uint8_t rx_buffer[RX_BUFFER_SIZE];
extern volatile uint8_t rx_index;

#define RX_BUFFER_SIZE_USART2 100
extern uint8_t rx_buffer_usart2[RX_BUFFER_SIZE_USART2];
extern volatile uint8_t rx_index_usart2;
extern volatile uint8_t usart2_cmd_ready;

/* USER CODE END Private defines */

void MX_USART1_UART_Init(void);
void MX_USART2_UART_Init(void);

/* USER CODE BEGIN Prototypes */

/**
  * @brief  获取接收到的字符串
  * @param  buffer: 输出缓冲区
  * @param  size: 缓冲区大小
  * @retval 接收的字节数
  */
uint8_t USART1_GetReceivedData(uint8_t *buffer, uint8_t size);

/**
  * @brief  获取USART2接收到的字符串
  * @param  buffer: 输出缓冲区
  * @param  size: 缓冲区大小
  * @retval 接收的字节数
  */
uint8_t USART2_GetReceivedData(uint8_t *buffer, uint8_t size);

/**
  * @brief  UART命令处理函数类型定义
  * @param  cmd: 接收到的命令数据
  * @param  cmd_len: 命令长度
  * @retval None
  */
typedef void (*USART_CommandHandler_t)(uint8_t *cmd, uint8_t cmd_len);

/**
  * @brief  注册USART1命令处理函数
  * @param  handler: 命令处理函数指针
  * @retval None
  */
void USART1_RegisterCommandHandler(USART_CommandHandler_t handler);

/**
  * @brief  处理USART1命令（在main循环中调用）
  * @retval None
  */
void USART1_Process(void);

/**
  * @brief  处理USART2命令（在main循环中调用）
  * @retval None
  */
void USART2_Process(void);

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __USART_H__ */


/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    spi.h
  * @brief   This file contains all the function prototypes for
  *          the spi.c file
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
#ifndef __SPI_H__
#define __SPI_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

extern SPI_HandleTypeDef hspi2;

/* USER CODE BEGIN Private defines */

#define SPI2_MSG_MAX 128U

/* USER CODE END Private defines */

void MX_SPI2_Init(void);

/* USER CODE BEGIN Prototypes */

void SPI2_SlaveComm_Start(void);
uint8_t SPI2_SlaveComm_GetCmd(uint8_t *out, uint16_t maxlen);
void SPI2_SlaveComm_SetResponse(const uint8_t *data, uint16_t len);
uint8_t SPI2_SlaveComm_HasPendingCmd(void);
uint8_t SPI2_SlaveComm_IsResponseReady(void);
void SPI2_SlaveIRQHandler(void);
void SPI2_SlaveComm_Poll(void);
HAL_StatusTypeDef HAL_SPI_Enable(SPI_HandleTypeDef *hspi);

/**
  * @brief  SPI命令处理函数类型定义
  * @param  cmd: 接收到的命令数据
  * @param  cmd_len: 命令长度
  * @param  response: 响应数据缓冲区
  * @param  response_len: 响应长度输出指针
  * @param  response_maxlen: 响应缓冲区最大长度
  * @retval None
  */
typedef void (*SPI_CommandHandler_t)(uint8_t *cmd, uint8_t cmd_len, 
                                      uint8_t *response, uint8_t *response_len, 
                                      uint16_t response_maxlen);

/**
  * @brief  注册SPI命令处理函数
  * @param  handler: 命令处理函数指针
  * @retval None
  */
void SPI2_RegisterCommandHandler(SPI_CommandHandler_t handler);

/**
  * @brief  处理SPI命令（在main循环中调用）
  * @retval None
  */
void SPI2_Process(void);

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __SPI_H__ */

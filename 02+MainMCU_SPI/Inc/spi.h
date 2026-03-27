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
extern WWDG_HandleTypeDef hwwdg;  // 添加看门狗句柄的外部声明

/* USER CODE BEGIN Private defines */

/* CS引脚定义 */
#define SPI2_CS_PIN     GPIO_PIN_12
#define SPI2_CS_PORT    GPIOB

/* USER CODE END Private defines */

void MX_SPI2_Init(void);

/* USER CODE BEGIN Prototypes */

/**
  * @brief  SPI2传输数据
  * @param  tx_data: 要发送的数据
  * @param  rx_data: 接收数据缓冲区
  * @param  size: 数据大小
  * @retval HAL状态
  */
HAL_StatusTypeDef SPI2_TransmitReceive(uint8_t *tx_data, uint8_t *rx_data, uint16_t size);

/**
  * @brief  SPI2：同一次片选内"先发送，再额外打时钟接收"
  * @note   典型用法：发送 payload+"\r\n" 后，发送 dummy(len+1) 并接收 payload+'+'
  * @param  tx_data: 要先发送的数据（例如 payload+"\r\n"）
  * @param  tx_size: 先发送的数据长度（字节）
  * @param  rx_data: 接收缓冲区（接收发生在 dummy 阶段）
  * @param  rx_size: 需要接收的字节数（也会发送同等长度的 dummy）
  * @retval HAL状态
  */
HAL_StatusTypeDef SPI2_TransmitThenReceive(uint8_t *tx_data, uint16_t tx_size,
                                          uint8_t *rx_data, uint16_t rx_size);

/**
  * @brief  SPI2发送数据
  * @param  data: 要发送的数据
  * @param  size: 数据大小
  * @retval HAL状态
  */
HAL_StatusTypeDef SPI2_Transmit(uint8_t *data, uint16_t size);

/**
  * @brief  选择SPI设备
  * @param  select: 1-选择设备（低电平），0-取消选择（高电平）
  * @retval None
  */
void SPI2_SelectDevice(uint8_t select);

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __SPI_H__ */

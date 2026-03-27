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

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __SPI_H__ */

/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    led.h
  * @brief   This file contains all the function prototypes for
  *          the led.c file
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
#ifndef __LED_H__
#define __LED_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

/* USER CODE BEGIN Prototypes */

/**
  * @brief  初始化LED控制（使用SysTick翻转，不使用PWM）
  * @retval None
  */
void LED_Init(void);

/**
  * @brief  LED主处理函数（在main循环中调用，1秒周期）
  * @retval None
  */
void LED_Process(void);

/**
  * @brief  设置LED状态
  * @param  led3_on: LED3状态 1=亮, 0=灭
  * @param  led4_on: LED4状态 1=亮, 0=灭
  * @retval None
  */
void LED_SetState(uint8_t led3_on, uint8_t led4_on);

/**
  * @brief  翻转LED状态
  * @retval None
  */
void LED_Toggle(void);

/**
  * @brief  获取LED3状态
  * @retval uint8_t: 1=亮, 0=灭
  */
uint8_t LED_GetState3(void);

/**
  * @brief  获取LED4状态
  * @retval uint8_t: 1=亮, 0=灭
  */
uint8_t LED_GetState4(void);

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __LED_H__ */

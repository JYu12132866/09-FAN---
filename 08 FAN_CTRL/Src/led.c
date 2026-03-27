/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    led.c
  * @brief   This file provides code for the configuration
  *          of the LED GPIO instances.
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
#include "led.h"
#include "gpio.h"
#include <stdint.h>

/* USER CODE BEGIN 0 */

/* LED引脚定义 - 根据实际硬件修改 */
#define LED3_GPIO_Port  GPIOB
#define LED3_Pin        GPIO_PIN_8
#define LED4_GPIO_Port  GPIOB
#define LED4_Pin        GPIO_PIN_9

/* LED状态 */
static uint8_t led3_state = 0;
static uint8_t led4_state = 0;
static uint32_t last_toggle_tick = 0;

/* USER CODE END 0 */

/* USER CODE BEGIN 1 */

/**
  * @brief  初始化LED控制（使用GPIO翻转，不使用PWM）
  * @retval None
  */
void LED_Init(void)
{
    /* 配置LED引脚为输出 */
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    __HAL_RCC_GPIOB_CLK_ENABLE();
    
    GPIO_InitStruct.Pin = LED3_Pin | LED4_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    
    /* 初始状态：LED3亮，LED4灭 */
    led3_state = 1;
    led4_state = 0;
    HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LED4_GPIO_Port, LED4_Pin, GPIO_PIN_RESET);
    
    last_toggle_tick = HAL_GetTick();
}

/**
  * @brief  LED主处理函数（在main循环中调用，1秒周期）
  * @retval None
  */
void LED_Process(void)
{
    if (HAL_GetTick() - last_toggle_tick >= 1000U)
    {
        last_toggle_tick += 1000U;
        LED_Toggle();
    }
}

/**
  * @brief  设置LED状态
  * @param  led3_on: LED3状态 1=亮, 0=灭
  * @param  led4_on: LED4状态 1=亮, 0=灭
  * @retval None
  */
void LED_SetState(uint8_t led3_on, uint8_t led4_on)
{
    led3_state = led3_on ? 1 : 0;
    led4_state = led4_on ? 1 : 0;
    
    HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, led3_state ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED4_GPIO_Port, LED4_Pin, led4_state ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

/**
  * @brief  翻转LED状态
  * @retval None
  */
void LED_Toggle(void)
{
    if (led3_state)
    {
        LED_SetState(0, 1);
    }
    else
    {
        LED_SetState(1, 0);
    }
}

/**
  * @brief  获取LED3状态
  * @retval uint8_t: 1=亮, 0=灭
  */
uint8_t LED_GetState3(void)
{
    return led3_state;
}

/**
  * @brief  获取LED4状态
  * @retval uint8_t: 1=亮, 0=灭
  */
uint8_t LED_GetState4(void)
{
    return led4_state;
}

/* USER CODE END 1 */

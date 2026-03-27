/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    tim.h
  * @brief   This file contains all the function prototypes for
  *          the tim.c file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
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
#ifndef __TIM_H__
#define __TIM_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

/* USER CODE BEGIN Includes */
#include "main.h"

/* USER CODE END Includes */


/* USER CODE BEGIN Private defines */
#define Tim_PWM_DC_int 0
#define Tim_PWM_DC_stat 99
#define Tim_PWM_DC_end 999

#define FAN_PWM_DC_NORMAL (Tim_PWM_DC_end * 70 / 100)
#define FAN_PWM_DC_ECO    (Tim_PWM_DC_end * 30 / 100)
#define FAN_PWM_DC_HI_ALT (Tim_PWM_DC_end * 99 / 100)

#define FAN_PWM_60DC (Tim_PWM_DC_end * 60 / 100)
#define FAN_PWM_50DC (Tim_PWM_DC_end * 50 / 100)

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;
extern TIM_HandleTypeDef htim6;
extern TIM_HandleTypeDef htim9;
extern uint16_t TEC_PWM_DC;

extern  volatile uint16_t loopCnt_1ms;
extern  volatile uint16_t loopCnt_10ms;
extern  volatile uint16_t loopCnt_50ms;
extern  volatile uint16_t loopCnt_100ms;
extern  volatile uint16_t loopCnt_500ms;
extern  volatile uint16_t loopCnt_1000ms;

extern  uint8_t loopFlag_1ms;
extern  uint8_t loopFlag_10ms;
extern  uint8_t loopFlag_50ms;
extern  uint8_t loopFlag_100ms;
extern  uint8_t loopFlag_500ms;
extern  uint8_t loopFlag_1000ms;


// extern uint16_t XML_timer;
// extern uint16_t timer ;
extern uint8_t TMP_timer;
extern uint8_t Fan_timer;
extern uint8_t OE_cnt;
extern uint8_t OE_timer;
extern uint8_t Soft_cnt;
extern uint8_t OE_timer_cnt;

/* USER CODE END Private defines */

void MX_TIM1_Init(void);
void MX_TIM2_Init(void);
void MX_TIM3_Init(void);
void MX_TIM4_Init(void);
void MX_TIM6_Init(void);
void MX_TIM9_Init(void);

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* USER CODE BEGIN Prototypes */
void Timer_Pwm_enable(void);
void Timer_Base_enable(void);
void TIM_PWM_Capture(void);
// void TIM_PWM_EN(void);
void FAN_PWM_Set(uint16_t FAN_PWM);
//void FAN_DC_Set(uint8_t FAN_Duty);
/* USER CODE END Prototypes */

void initTimerPeroid(void);

#ifdef __cplusplus
}
#endif

#endif /* __TIM_H__ */


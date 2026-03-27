/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    tim.c
 * @brief   This file provides code for the configuration
 *          of the TIM instances.
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
/* Includes ------------------------------------------------------------------*/
#include "tim.h"

/* USER CODE BEGIN 0 */
#include "adc.h"
#include "Motor.h"
#include "error.h"
#include "main.h"
#include "i2c.h"
#include <string.h>
#include "udp_demo.h"
// #include "PID.h"
#include "lwip_comm.h"
#include "lan8720.h"
#include "fanctl.h"
#include "error.h"
#include "pjlink.h"
#include "rt_param.h"


_Bool SoftStart_FLAG = 0;


/**TIM1 GPIO Configuration
PE9     ------> TIM1_CH1  FAN1_PWM_Pin    FAN1  FAN5
PE11     ------> TIM1_CH2  FAN2_PWM_Pin   FAN3
PE13     ------> TIM1_CH3 FAN8_PWM_Pin   FAN_L3 FAN_L4
PE14     ------> TIM1_CH4 FAN9_PWM_Pin   FAN_B1 FAN_B2
*/
#define TIM1_Period 1000 // 1khz
uint16_t FAN1_PWM_DC = 99;
uint16_t FAN2_PWM_DC = 99;
uint16_t FAN8_PWM_DC = 99;
uint16_t FAN9_PWM_DC = 99;

/**TIM2 GPIO Configuration
PA0-WKUP     ------> TIM2_CH1 FAN3_PWM_Pin   FAN2 FAN6
PB3     ------> TIM2_CH2      FAN4_PWM_Pin
*/
#define TIM2_Period 1000   // 1khz
uint16_t FAN3_PWM_DC = 99; //(TIM2_Period*50/100);
uint16_t FAN4_PWM_DC = 99; //(TIM2_Period*50/100);

/**TIM3 GPIO Configuration
PB4     ------> TIM3_CH1 FAN6_PWM_Pin  FAN_L1 FAN_L2
PB5     ------> TIM3_CH2 FAN7_PWM_Pin  FAN_W2 FAN_W1
//PB1     ------> TIM3_CH3 MOTOR3_STEP_Pin
*/


uint16_t FAN6_PWM_DC = 25; //(TIM3_Period*50/100);
uint16_t FAN7_PWM_DC = 25; // (TIM3_Period*50/100);

/**TIM4 GPIO Configuration
PD12     ------> TIM4_CH1 TEC_PWM_Pin
PD13     ------> TIM4_CH2 FAN10_PWM_Pin
//PB8     ------> TIM4_CH3 MOTOR2_STEP
PB9     ------> TIM4_CH4  FAN5_PWM_Pin
*/


uint16_t TEC_PWM_DC = 0; // (TIM4_Period*50/100);
uint16_t FAN10_PWM_DC = 25;
uint16_t FAN5_PWM_DC = 25; // TIM4_Period*50/100;

uint16_t TIM3_Period = 200;       
uint16_t TIM4_Period = 200;       
uint16_t TIM9_Period = 50;     
uint16_t MOTOR_STEP_Pulse  = 0 ;

// #define timeup 200;
// uint16_t XML_timer = 300;
// uint16_t timer = 300;
uint8_t TMP_timer;
uint8_t Fan_timer;
uint8_t OE_cnt;
uint8_t OE_timer;
uint8_t Soft_cnt;
uint8_t OE_timer_cnt = IR_Rep2;
uint8_t Gate_SW = 1;
// extern _Bool Sys_start;
// extern uint8_t FAN_DC;
uint16_t PWM_DC = FAN_PWM_DC_NORMAL;

/* USER CODE END 0 */
// uint8_t counter;
TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;
TIM_HandleTypeDef htim6;
TIM_HandleTypeDef htim9;

/* TIM1 init function */
void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */
  // PE9     ------> TIM1_CH1  FAN1_PWM_Pin
  // PE11     ------> TIM1_CH2  FAN2_PWM_Pin
  // PE13     ------> TIM1_CH3 FAN8_PWM_Pin
  // PE14     ------> TIM1_CH4 FAN9_PWM_Pin
  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 42 - 1;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = TIM1_Period - 1;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = Tim_PWM_DC_end;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_ENABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  __HAL_TIM_DISABLE_OCxPRELOAD(&htim1, TIM_CHANNEL_1);

  sConfigOC.Pulse = Tim_PWM_DC_end;
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  __HAL_TIM_DISABLE_OCxPRELOAD(&htim1, TIM_CHANNEL_2);

  sConfigOC.Pulse = Tim_PWM_DC_end;
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }
  __HAL_TIM_DISABLE_OCxPRELOAD(&htim1, TIM_CHANNEL_3);

  sConfigOC.Pulse = Tim_PWM_DC_end;
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
  {
    Error_Handler();
  }
  __HAL_TIM_DISABLE_OCxPRELOAD(&htim1, TIM_CHANNEL_4);
  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */
  HAL_TIM_MspPostInit(&htim1);
}
/* TIM2 init function */
void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM2_Init 1 */
  // PA0-WKUP     ------> TIM2_CH1 FAN3_PWM_Pin
  // PB3     ------> TIM2_CH2 FAN4_PWM_Pin
  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 42 - 1;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = TIM2_Period - 1;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = Tim_PWM_DC_end;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_ENABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  __HAL_TIM_DISABLE_OCxPRELOAD(&htim2, TIM_CHANNEL_1);

  sConfigOC.Pulse = Tim_PWM_DC_end;
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  __HAL_TIM_DISABLE_OCxPRELOAD(&htim2, TIM_CHANNEL_2);
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */
  HAL_TIM_MspPostInit(&htim2);
}
/* TIM3 init function */
void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM3_Init 1 */
  // PB4     ------> TIM3_CH1 FAN6_PWM_Pin
  // PB5     ------> TIM3_CH2 FAN7_PWM_Pin
  // PB1     ------> TIM3_CH4 MOTOR3_STEP_Pin
  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 42 - 1;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  htim3.Init.Period = TIM3_Period - 1;
  
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = Tim_PWM_DC_end;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_ENABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  __HAL_TIM_DISABLE_OCxPRELOAD(&htim3, TIM_CHANNEL_1);

  sConfigOC.Pulse = Tim_PWM_DC_end;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  __HAL_TIM_DISABLE_OCxPRELOAD(&htim3, TIM_CHANNEL_2);
  /* USER CODE BEGIN TIM3_Init 2 */
#if (Projector_Model == LP92SLB || Projector_Model == NP72BN)
  sConfigOC.Pulse = Tim_PWM_DC_end;
#else
  sConfigOC.Pulse = MOTOR_STEP_Pulse;
#endif
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
  {
    Error_Handler();
  }
  __HAL_TIM_DISABLE_OCxPRELOAD(&htim3, TIM_CHANNEL_4);
  /* USER CODE END TIM3_Init 2 */
  HAL_TIM_MspPostInit(&htim3);
}
/* TIM4 init function */
void MX_TIM4_Init(void)
{

  /* USER CODE BEGIN TIM4_Init 0 */

  /* USER CODE END TIM4_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM4_Init 1 */
  // PD12     ------> TIM4_CH1 TEC_PWM_Pin
  // PD13     ------> TIM4_CH2 FAN10_PWM_Pin
  // PB8     ------> TIM4_CH3 MOTOR2_STEP
  // PB9     ------> TIM4_CH4 FAN5_PWM_Pin
  /* USER CODE END TIM4_Init 1 */
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 42 - 1;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  htim4.Init.Period = TIM4_Period - 1;

  if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = TEC_PWM_DC;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_ENABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  __HAL_TIM_DISABLE_OCxPRELOAD(&htim4, TIM_CHANNEL_1);

  sConfigOC.Pulse = Tim_PWM_DC_end;
  if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  __HAL_TIM_DISABLE_OCxPRELOAD(&htim4, TIM_CHANNEL_2);

#if (Projector_Model == LP92SLB || Projector_Model == NP72BN)
  sConfigOC.Pulse = Tim_PWM_DC_end;
  if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }
#else
  sConfigOC.Pulse = MOTOR_STEP_Pulse;
  if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }
#endif
  __HAL_TIM_DISABLE_OCxPRELOAD(&htim4, TIM_CHANNEL_3);

  sConfigOC.Pulse = Tim_PWM_DC_end;
  if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
  {
    Error_Handler();
  }
  __HAL_TIM_DISABLE_OCxPRELOAD(&htim4, TIM_CHANNEL_4);
  /* USER CODE BEGIN TIM4_Init 2 */

  /* USER CODE END TIM4_Init 2 */
  HAL_TIM_MspPostInit(&htim4);
}
/* TIM6 init function */
void MX_TIM6_Init(void)
{

  /* USER CODE BEGIN TIM6_Init 0 */

  /* USER CODE END TIM6_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM6_Init 1 */

// ** ˵������ʱʱ��=(Ԥ��Ƶ��+1��*������ֵ+1) /TIM6ʱ��(42MHz)����λ(s)
//    �������ʱ��t=(4200*10)/42000000s=0.001s

  /* USER CODE END TIM6_Init 1 */
  htim6.Instance = TIM6;

  // htim6.Init.Prescaler = 42000 - 1;
  // htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
  // htim6.Init.Period = 100 - 1; // 0.1s

  htim6.Init.Prescaler = 4200 - 1;
  htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
  // htim6.Init.Period = 100 - 1; // 0.1s
  htim6.Init.Period = 10 - 1; //1ms
  htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM6_Init 2 */

  /* USER CODE END TIM6_Init 2 */
}
/* TIM9 init function */
void MX_TIM9_Init(void)
{

  /* USER CODE BEGIN TIM9_Init 0 */

  /* USER CODE END TIM9_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM9_Init 1 */
  // PE5     ------> TIM9_CH1 MOTOR2_STEP
  // PE6     ------> TIM9_CH2 MOTOR3_STEP
  /* USER CODE END TIM9_Init 1 */
  htim9.Instance = TIM9;
  htim9.Init.Prescaler = 42-1;
  htim9.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim9.Init.Period = TIM9_Period - 1;
  // htim9.Init.Period = 20000 - 1;
  htim9.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim9.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim9) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim9, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim9) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim9, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_ENABLE;
  sConfigOC.Pulse = MOTOR_STEP_Pulse;

  if (HAL_TIM_PWM_ConfigChannel(&htim9, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  __HAL_TIM_DISABLE_OCxPRELOAD(&htim9, TIM_CHANNEL_1);

  if (HAL_TIM_PWM_ConfigChannel(&htim9, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  __HAL_TIM_DISABLE_OCxPRELOAD(&htim9, TIM_CHANNEL_2);
  /* USER CODE BEGIN TIM9_Init 2 */

  /* USER CODE END TIM9_Init 2 */
  HAL_TIM_MspPostInit(&htim9);
}


void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *tim_baseHandle)
{

  //  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if (tim_baseHandle->Instance == TIM1)
  {
    /* USER CODE BEGIN TIM1_MspInit 0 */

    /* USER CODE END TIM1_MspInit 0 */
    /* TIM1 clock enable */
    __HAL_RCC_TIM1_CLK_ENABLE();

    /* TIM1 interrupt Init */
    HAL_NVIC_SetPriority(TIM1_BRK_TIM9_IRQn, 2, 1);
    HAL_NVIC_EnableIRQ(TIM1_BRK_TIM9_IRQn);
    /* USER CODE BEGIN TIM1_MspInit 1 */

    /* USER CODE END TIM1_MspInit 1 */
  }
  else if (tim_baseHandle->Instance == TIM2)
  {
    /* USER CODE BEGIN TIM2_MspInit 0 */

    /* USER CODE END TIM2_MspInit 0 */
    /* TIM2 clock enable */
    __HAL_RCC_TIM2_CLK_ENABLE();

    /* TIM2 interrupt Init */
    HAL_NVIC_SetPriority(TIM2_IRQn, 2, 1);
    HAL_NVIC_EnableIRQ(TIM2_IRQn);
    /* USER CODE BEGIN TIM2_MspInit 1 */

    /* USER CODE END TIM2_MspInit 1 */
  }
  else if (tim_baseHandle->Instance == TIM3)
  {
    /* USER CODE BEGIN TIM3_MspInit 0 */

    /* USER CODE END TIM3_MspInit 0 */
    /* TIM3 clock enable */
    __HAL_RCC_TIM3_CLK_ENABLE();

    /* TIM3 interrupt Init */
    HAL_NVIC_SetPriority(TIM3_IRQn, 2, 1);
    HAL_NVIC_EnableIRQ(TIM3_IRQn);
    /* USER CODE BEGIN TIM3_MspInit 1 */

    /* USER CODE END TIM3_MspInit 1 */
  }
  else if (tim_baseHandle->Instance == TIM4)
  {
    /* USER CODE BEGIN TIM4_MspInit 0 */

    /* USER CODE END TIM4_MspInit 0 */
    /* TIM4 clock enable */
    __HAL_RCC_TIM4_CLK_ENABLE();

    /* TIM4 interrupt Init */
    HAL_NVIC_SetPriority(TIM4_IRQn, 2, 1);
    HAL_NVIC_EnableIRQ(TIM4_IRQn);
    /* USER CODE BEGIN TIM4_MspInit 1 */

    /* USER CODE END TIM4_MspInit 1 */
  }
  else if (tim_baseHandle->Instance == TIM6)
  {
    /* USER CODE BEGIN TIM6_MspInit 0 */

    /* USER CODE END TIM6_MspInit 0 */
    /* TIM6 clock enable */
    __HAL_RCC_TIM6_CLK_ENABLE();

    /* TIM6 interrupt Init */
    HAL_NVIC_SetPriority(TIM6_DAC_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);
    /* USER CODE BEGIN TIM6_MspInit 1 */
    // HAL_NVIC_SetPriority(TIM6_DAC_IRQn, 2, 0);
    // HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);
    /* USER CODE END TIM6_MspInit 1 */
  }
  else if (tim_baseHandle->Instance == TIM9)
  {
    /* USER CODE BEGIN TIM9_MspInit 0 */

    /* USER CODE END TIM9_MspInit 0 */
    /* TIM6 clock enable */
    __HAL_RCC_TIM9_CLK_ENABLE();

    /* TIM9 interrupt Init */
    HAL_NVIC_SetPriority(TIM1_BRK_TIM9_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(TIM1_BRK_TIM9_IRQn);
    /* USER CODE BEGIN TIM9_MspInit 1 */

    /* USER CODE END TIM9_MspInit 1 */
  }
}


void HAL_TIM_MspPostInit(TIM_HandleTypeDef *timHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if (timHandle->Instance == TIM1)
  {
    /* USER CODE BEGIN TIM1_MspPostInit 0 */

    /* USER CODE END TIM1_MspPostInit 0 */
    __HAL_RCC_GPIOE_CLK_ENABLE();
    /**TIM1 GPIO Configuration
    PE9     ------> TIM1_CH1
    PE11     ------> TIM1_CH2
    PE13     ------> TIM1_CH3
    PE14     ------> TIM1_CH4
    */
    GPIO_InitStruct.Pin = FAN1_PWM_Pin | FAN2_PWM_Pin | FAN8_PWM_Pin | FAN9_PWM_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF1_TIM1;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

    /* USER CODE BEGIN TIM1_MspPostInit 1 */

    /* USER CODE END TIM1_MspPostInit 1 */
  }
  else if (timHandle->Instance == TIM2)
  {
    /* USER CODE BEGIN TIM2_MspPostInit 0 */

    /* USER CODE END TIM2_MspPostInit 0 */

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**TIM2 GPIO Configuration
    PA0-WKUP     ------> TIM2_CH1
    PB3     ------> TIM2_CH2
    */
    GPIO_InitStruct.Pin = FAN3_PWM_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF1_TIM2;
    HAL_GPIO_Init(FAN3_PWM_GPIO_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = FAN4_PWM_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF1_TIM2;
    HAL_GPIO_Init(FAN4_PWM_GPIO_Port, &GPIO_InitStruct);

    /* USER CODE BEGIN TIM2_MspPostInit 1 */

    /* USER CODE END TIM2_MspPostInit 1 */
  }
  else if (timHandle->Instance == TIM3)
  {
    /* USER CODE BEGIN TIM3_MspPostInit 0 */

    /* USER CODE END TIM3_MspPostInit 0 */

    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**TIM3 GPIO Configuration
    PB4     ------> TIM3_CH1
    PB5     ------> TIM3_CH2
     PB0     ------> TIM3_CH3
    */
#if (Projector_Model == LP92SLB || Projector_Model == NP72BN)
// no operation
#else
    GPIO_InitStruct.Pin = MOTOR3_STEP_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
#endif

    GPIO_InitStruct.Pin = FAN6_PWM_Pin | FAN7_PWM_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* USER CODE BEGIN TIM3_MspPostInit 1 */

    /* USER CODE END TIM3_MspPostInit 1 */
  }
  else if (timHandle->Instance == TIM4)
  {
    /* USER CODE BEGIN TIM4_MspPostInit 0 */

    /* USER CODE END TIM4_MspPostInit 0 */

    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**TIM4 GPIO Configuration
    PD12     ------> TIM4_CH1
    PD13     ------> TIM4_CH2
    PD8    ------> TIM4_CH3
    PB9     ------> TIM4_CH4
    */
    GPIO_InitStruct.Pin = TEC_PWM_Pin | FAN10_PWM_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF2_TIM4;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = FAN5_PWM_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF2_TIM4;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

#if (Projector_Model == LP92SLB || Projector_Model == NP72BN)
    // No operation
#else

    GPIO_InitStruct.Pin = MOTOR2_STEP_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF2_TIM4;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
#endif
    /* USER CODE BEGIN TIM4_MspPostInit 1 */

    /* USER CODE END TIM4_MspPostInit 1 */
  }
#if (Projector_Model == LP92SLB || Projector_Model == NP72BN)
  else if (timHandle->Instance == TIM9)
  {
    /* USER CODE BEGIN TIM9_MspPostInit 0 */

    /* USER CODE END TIM9_MspPostInit 0 */

    __HAL_RCC_GPIOE_CLK_ENABLE();
    /**TIM9 GPIO Configuration
    PE5     ------> TIM9_CH1
    PE6     ------> TIM9_CH2
    */
    GPIO_InitStruct.Pin = MOTOR2_STEP_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF3_TIM9;
    HAL_GPIO_Init(MOTOR2_STEP_GPIO_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = MOTOR3_STEP_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF3_TIM9;
    HAL_GPIO_Init(MOTOR3_STEP_GPIO_Port, &GPIO_InitStruct);
    /* USER CODE BEGIN TIM4_MspPostInit 1 */

    /* USER CODE END TIM4_MspPostInit 1 */
  }
#endif
}


void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef *tim_baseHandle)
{

  if (tim_baseHandle->Instance == TIM1)
  {
    /* USER CODE BEGIN TIM1_MspDeInit 0 */

    /* USER CODE END TIM1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_TIM1_CLK_DISABLE();

    /* TIM1 interrupt Deinit */
    /* USER CODE BEGIN TIM1:TIM1_BRK_TIM9_IRQn disable */
    /**
     * Uncomment the line below to disable the "TIM1_BRK_TIM9_IRQn" interrupt
     * Be aware, disabling shared interrupt may affect other IPs
     */
    /* HAL_NVIC_DisableIRQ(TIM1_BRK_TIM9_IRQn); */
    /* USER CODE END TIM1:TIM1_BRK_TIM9_IRQn disable */

    /* USER CODE BEGIN TIM1_MspDeInit 1 */

    /* USER CODE END TIM1_MspDeInit 1 */
  }
  else if (tim_baseHandle->Instance == TIM2)
  {
    /* USER CODE BEGIN TIM2_MspDeInit 0 */

    /* USER CODE END TIM2_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_TIM2_CLK_DISABLE();

    /* TIM2 interrupt Deinit */
    HAL_NVIC_DisableIRQ(TIM2_IRQn);
    /* USER CODE BEGIN TIM2_MspDeInit 1 */

    /* USER CODE END TIM2_MspDeInit 1 */
  }
  else if (tim_baseHandle->Instance == TIM3)
  {
    /* USER CODE BEGIN TIM3_MspDeInit 0 */

    /* USER CODE END TIM3_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_TIM3_CLK_DISABLE();

    /* TIM3 interrupt Deinit */
    HAL_NVIC_DisableIRQ(TIM3_IRQn);
    /* USER CODE BEGIN TIM3_MspDeInit 1 */

    /* USER CODE END TIM3_MspDeInit 1 */
  }
  else if (tim_baseHandle->Instance == TIM4)
  {
    /* USER CODE BEGIN TIM4_MspDeInit 0 */

    /* USER CODE END TIM4_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_TIM4_CLK_DISABLE();

    /* TIM4 interrupt Deinit */
    HAL_NVIC_DisableIRQ(TIM4_IRQn);
    /* USER CODE BEGIN TIM4_MspDeInit 1 */

    /* USER CODE END TIM4_MspDeInit 1 */
  }
  else if (tim_baseHandle->Instance == TIM6)
  {
    /* USER CODE BEGIN TIM6_MspDeInit 0 */

    /* USER CODE END TIM6_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_TIM6_CLK_DISABLE();

    /* TIM6 interrupt Deinit */
    HAL_NVIC_DisableIRQ(TIM6_DAC_IRQn);
    /* USER CODE BEGIN TIM6_MspDeInit 1 */

    /* USER CODE END TIM6_MspDeInit 1 */
  }
  else if (tim_baseHandle->Instance == TIM9)
  {
    /* USER CODE BEGIN TIM9_MspDeInit 0 */

    /* USER CODE END TIM9_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_TIM9_CLK_DISABLE();

    /* TIM3 interrupt Deinit */
    // HAL_NVIC_DisableIRQ(TIM1_BRK_TIM9_IRQn);
    /* USER CODE BEGIN TIM9_MspDeInit 1 */

    /* USER CODE END TIM9_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

void Timer_Base_enable(void)
{
  HAL_TIM_Base_Start(&htim1);
  HAL_TIM_Base_Start(&htim2);
  HAL_TIM_Base_Start(&htim3);
  HAL_TIM_Base_Start(&htim4);
#if (Projector_Model == LP92SLB || Projector_Model == NP72BN)
  HAL_TIM_Base_Start(&htim9);
#endif
}


void Timer_Pwm_enable(void)
{
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4);

  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);

  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);

  HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_2);
  HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_4);

#if (Projector_Model == LP92SLB || Projector_Model == NP72BN)
  // HAL_TIM_PWM_Start(&htim9, TIM_CHANNEL_1);
  // HAL_TIM_PWM_Start(&htim9, TIM_CHANNEL_2);
#endif
}


// void TIM_PWM_EN(void)
// {
//   if (Work_Stat)
//   {
//     FAN_PWM_En = 0x41;
//   }
//   else
//   {
//     FAN_PWM_En = 0x14;
//   }
// }


void Soft_PWM_stop(void)
{
  if (FAN1_PWM_DC > Tim_PWM_DC_stat)
  {
    FAN1_PWM_DC -= 50;
    FAN2_PWM_DC = FAN1_PWM_DC;
    FAN3_PWM_DC = FAN1_PWM_DC;
    FAN4_PWM_DC = FAN1_PWM_DC;
    FAN5_PWM_DC = FAN1_PWM_DC;
    FAN6_PWM_DC = FAN1_PWM_DC;
    FAN7_PWM_DC = FAN1_PWM_DC;
    FAN8_PWM_DC = FAN1_PWM_DC;
    FAN9_PWM_DC = FAN1_PWM_DC;
    FAN10_PWM_DC = FAN1_PWM_DC;
  }
  else
  {
    FAN1_PWM_DC = Tim_PWM_DC_int;
    FAN2_PWM_DC = Tim_PWM_DC_int;
    FAN3_PWM_DC = Tim_PWM_DC_int;
    FAN4_PWM_DC = Tim_PWM_DC_int;
    FAN5_PWM_DC = Tim_PWM_DC_int;
    FAN6_PWM_DC = Tim_PWM_DC_int;
    FAN7_PWM_DC = Tim_PWM_DC_int;
    FAN8_PWM_DC = Tim_PWM_DC_int;
    FAN9_PWM_DC = Tim_PWM_DC_int;
    FAN10_PWM_DC = Tim_PWM_DC_int;
    // FAN_PWM_En = 0;
    SoftStart_FLAG = 0;
    printf("FAN_PWM_stop\r\n");
  }
  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, FAN1_PWM_DC);
  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, FAN2_PWM_DC);
  __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, FAN3_PWM_DC);
  __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, FAN4_PWM_DC);
  __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_4, FAN5_PWM_DC);
  __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, FAN6_PWM_DC);
  __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, FAN7_PWM_DC);
  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, FAN8_PWM_DC);
  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, FAN9_PWM_DC);
  __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_2, FAN10_PWM_DC);

  /******************************************************/
}


void FAN_PWM_Set(uint16_t FAN_PWM)
{
  FAN1_PWM_DC = FAN_PWM;
  FAN2_PWM_DC = FAN_PWM;
  FAN3_PWM_DC = FAN_PWM;
  FAN4_PWM_DC = FAN_PWM;
  FAN5_PWM_DC = FAN_PWM;
  FAN6_PWM_DC = FAN_PWM;
  FAN7_PWM_DC = FAN_PWM;
  FAN8_PWM_DC = FAN_PWM;
  FAN9_PWM_DC = FAN_PWM;
  FAN10_PWM_DC = FAN_PWM;
  
  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, FAN1_PWM_DC);
  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, FAN2_PWM_DC);
  __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, FAN3_PWM_DC);
  __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, FAN4_PWM_DC);
  __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_4, FAN5_PWM_DC);
  __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, FAN6_PWM_DC);
  __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, FAN7_PWM_DC);
  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, FAN8_PWM_DC);
  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, FAN9_PWM_DC);
  __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_2, FAN10_PWM_DC);
  /******************************************************/
}


// void TEC_PWM_stop(void)
// {
//   if (TEC_PWM_DC > Tim_PWM_DC_stat)
//   {
//     TEC_PWM_DC -= 500;
//   }
//   else
//   {
//     TEC_PWM_DC = Tim_PWM_DC_int;
//     HAL_GPIO_WritePin(TEC_EN_GPIO_Port, TEC_EN_Pin, GPIO_PIN_RESET);
//     TEC_En = 0;
//     printf("TEC_PWM_stop\r\n");
//   }
//   __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, TEC_PWM_DC);
//   /******************************************************/
// }

volatile uint16_t loopCnt_1ms;
volatile uint16_t loopCnt_10ms;
volatile uint16_t loopCnt_50ms;
volatile uint16_t loopCnt_100ms;
volatile uint16_t loopCnt_500ms;
volatile uint16_t loopCnt_1000ms;

uint8_t loopFlag_1ms;
uint8_t loopFlag_10ms;
uint8_t loopFlag_50ms;
uint8_t loopFlag_100ms;
uint8_t loopFlag_500ms;
uint8_t loopFlag_1000ms;

/**
 * @brief timer6 1ms��ʱ��callback���� 
 * 
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) 
{
  /* Time slice �ܹ�������ʱ��Ƭ��ʱ */
  if (htim == (&htim6)) //1ms
  { 
    loopCnt_1ms++;

    if(++loopCnt_1ms * 1000 >= (1000)){ loopCnt_1ms = 0; loopFlag_1ms = 1;}
    if(++loopCnt_10ms * 100 >= (1000)){ loopCnt_10ms = 0; loopFlag_10ms = 1;}
    if(++loopCnt_50ms * 20 >= (1000)){ loopCnt_50ms = 0; loopFlag_50ms = 1;}
    if(++loopCnt_100ms * 10 >= (1000)){ loopCnt_100ms = 0; loopFlag_100ms = 1;}
    if(++loopCnt_500ms * 2 >= (1000)){ loopCnt_500ms = 0; loopFlag_500ms = 1;}
    if(++loopCnt_1000ms * 1 >= (1000)){ loopCnt_1000ms = 0; loopFlag_1000ms = 1;}
  }


#if 0 
  /* uart1��ʱ���յ���ʱ */
  /*
    uart1 ������Զ�Э��  
    +--------+--------+---------+----------+
    |  1byte |  1byte | 252byte |  1byte   |
    +--------+--------+---------+----------+
    | header | length | payload | checksum |
    +--------+--------+---------+----------+
                            |          
                            ��          
                  uart1 payload�Զ�Э��  
                  +--------+--------+
                  |  2byte | N byte |
                  +--------+--------+
                  | cmd    | param  |
                  +--------+--------+
                     __|__          
                    ��     �� 
          ��Ӧ��buffer    ָ���buffer
  */
  if (g_uart1_checkTick)
  {
      --g_uart1_checkTick;
      if (g_uart1_checkTick == 0)
      {
          if ((g_uart1_idx > 2) && (g_uart1_rx_buf[0] == 0XAA)) // ��������header
          {
              uint8_t length = g_uart1_rx_buf[1];
              uint16_t cmd = *(uint16_t *)(&g_uart1_rx_buf[2]);
              uint8_t checksum = g_uart1_rx_buf[g_uart1_idx - 1];
              if (checksum == simple_checksum(g_uart1_rx_buf, g_uart1_idx - 1)) // ���checksum
              {
                  if (cmd == 0x01) // ACK ��
                  {
                      if (g_uart1_ack_flag == 0)
                      {
                          memcpy(g_uart1_rx_ack_buf, g_uart1_rx_buf + 2, length); // �������ݵ�ack buffer
                          g_uart1_ack_len = length;
                          g_uart1_ack_flag = 1;
                      }
                  }
                  else // CMD��
                  {
                      memcpy(g_uart1_rx_cmd_buf, g_uart1_rx_buf + 2, length); // �������ݵ�cmd buffer
                      g_uart1_cmd_len = length;
                      g_uart1_cmd_flag = 1;
                  }
              }
              else
              {
                  printf("uart1 rx checksum err\n");
              }
          }
          else
          {
              printf("uart1 rx header err\n");
          }
          g_uart1_idx = 0 ;   //�������buffer����s
      }
  }

  #endif
}


/**
 * @brief 初始化定时器周期配置
 *
 * 根据投影仪型号和马达型号，初始化不同的定时器周期。
 *
 * 如果投影仪型号为LP92SLB或NP72BN，根据马达型号（新马达或旧马达）设置不同的定时器周期。
 * 否则，设置另一组定时器周期。
 *
 * @note 如果投影仪型号为LP92SLB或NP72BN，并且马达型号为SHIFT_MOTOR_DZ_24BYJ48_A14，
 *       则TIM3和TIM4周期用于风扇控制，TIM9周期用于位移马达控制（新马达）。
 *       否则，TIM3和TIM4周期同样用于风扇控制，TIM9周期用于位移马达控制（旧马达）。
 *
 *       如果投影仪型号不是LP92SLB或NP72BN，则根据马达型号设置另一组定时器周期。
 *       如果马达型号为SHIFT_MOTOR_DZ_24BYJ48_A14，则TIM3和TIM4周期用于位移马达水平控制（新马达），
 *       TIM9周期在仿真机下无用。否则，TIM3用于位移马达水平控制（旧马达），
 *       TIM4用于位移马达垂直控制（旧马达），TIM9周期在仿真机下同样无用。
 */
void initTimerPeroid(void)
{
#if(Projector_Model == LP92SLB || Projector_Model == NP72BN)
  if(rt_param.shift_motor_model == SHIFT_MOTOR_DZ_24BYJ48_A14)    // 适配新马达的参数
  {
    TIM3_Period = 1000 ;     // 用于风扇控制（固化）
    TIM4_Period = 1000 ;     // 用于风扇控制（固化）
    TIM9_Period = 200 ;      // 用于位移马达控制（新）
  }
  else{
    TIM3_Period = 1000 ;     //用于风扇控制（固化）
    TIM4_Period = 1000 ;     //用于风扇控制（固化）
    TIM9_Period = 50   ;     //用于位移马达控制（旧）  
  }                         // 适配旧马达的参数        
  MOTOR_STEP_Pulse =  (TIM9_Period * 50 / 100);
#else 
  if(rt_param.shift_motor_model == SHIFT_MOTOR_DZ_24BYJ48_A14)    // 适配新马达的参数
  {
    TIM3_Period = 200 ;       //用于位移马达水平控制（新）
    TIM4_Period = 200 ;       //用于位移马达水平控制（新）
    TIM9_Period = 50  ;       //仿真机下该参数无用（固化）
  }
  else{
    TIM3_Period = 50;        //用于位移马达水平控制（旧）
    TIM4_Period = 50;        //用于位移马达垂直控制（旧）
    TIM9_Period = 50;        //仿真机下该参数无用（固化）
  }                          // 适配旧马达的参数      
  MOTOR_STEP_Pulse =  (TIM4_Period * 50 / 100);
#endif
}

/* USER CODE END 1 */

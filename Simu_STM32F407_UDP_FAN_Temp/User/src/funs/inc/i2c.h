/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    i2c.h
 * @brief   This file contains all the function prototypes for
 *          the i2c.c file
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
#ifndef __I2C_H__
#define __I2C_H__

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "main.h"
/* USER CODE END Includes */

/* USER CODE BEGIN Private defines */
#define IIC_Timeout 2000
// extern uint8_t TEC_En;
// extern uint8_t FAN_PWM_En;
extern uint8_t Worktime[2];
extern I2C_HandleTypeDef hi2c1;
extern I2C_HandleTypeDef hi2c2;
extern HAL_StatusTypeDef I2C_status;
extern HAL_StatusTypeDef I2C_OLD_Sat;
extern uint8_t I2C_Busy2;
extern uint8_t I2C_Busy3;
extern uint8_t I2C_Busy1;
// extern _Bool I2C_Read_in;
extern _Bool I2C_IT_Trigger;
/* USER CODE END Private defines */

extern _Bool simuI2cFlag;

extern uint8_t DMD_Temp;

/* USER CODE BEGIN Prototypes */
void MX_I2C1_Init(void);
void MX_I2C2_Init(void);
void Reset_I2C(void);
void I2C_CMD_Process(void);
/* USER CODE END Prototypes */

void lensCtrlStep(uint8_t lensOps, uint8_t dir);


#ifdef __cplusplus
}
#endif

#endif /* __I2C_H__ */

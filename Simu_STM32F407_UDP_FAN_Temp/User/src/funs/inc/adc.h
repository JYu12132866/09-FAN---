/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    adc.h
 * @brief   This file contains all the function prototypes for
 *          the adc.c file
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
#ifndef __ADC_H__
#define __ADC_H__

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */
#include "math.h"

  /* USER CODE END Includes */

extern ADC_HandleTypeDef hadc1;

/* USER CODE BEGIN Private defines */
#if (Projector_Model == LP92SLB || Projector_Model == NP72BN)
#define ADC_NUMOFCHANNEL 9
#else 
#define ADC_NUMOFCHANNEL 5
#endif

#define OE_Shift_L 2.10f // voltage
#define OE_Shift_R 1.18f // voltage
#define OE_Shift_U 2.38f // 2.40f //voltage
#define OE_Shift_D 0.89f // 0.86f	 //voltage

// #define OE_Shift_L 1.8f // voltage
// #define OE_Shift_R 1.4f // voltage
// #define OE_Shift_U 2.0f // 2.40f //voltage
// #define OE_Shift_D 1.2f // 0.86f	 //voltage

#define OE_focus_L 3.01f // voltage
#define OE_focus_R 1.01f // voltage
#define OE_zoom_L 3.01f  // voltage
#define OE_zoom_R 1.01f  // voltage

#define ShiftH_Center 1.64f // voltage
#define ShiftV_Center 1.63f // voltage

#define OE_Focus 0x11
#define OE_Zoom 0x12
#define OE_Shift_LR 0x13
#define OE_Shift_UD 0x14
#define OE_Shift_RST 0x15

typedef struct {
  uint8_t dbg_switch;
  uint8_t dbg_temp_idx;
  unsigned char dbg_temp_val;
}TempCtl;

extern TempCtl temp_ctl;

extern const float Rp;                    // R�?�?敏电阻在T2常温下的标称阻值，10K的热敏电�?25℃的值为10K
extern const float T2;                    // 常温下的K氏温�?
extern const float Bx;                    // �?敏电阻的B值参考�?
extern const float Ka;                    // 绝�?�零度开尔文温度
extern const uint32_t ADC_pull_up_Res;    // 分压电阻阻�?
extern const float Calibration_parameter; // 校准参数

extern __IO float ADC_ConvertedValueLocal[];
extern float OE_Shift_LR_AVE[ADC_NUMOFCHANNEL];
extern float OE_Shift_UD_AVE[ADC_NUMOFCHANNEL];
extern float OE_Focus_LR_AVE[ADC_NUMOFCHANNEL];
extern float OE_Zoom_LR_AVE[ADC_NUMOFCHANNEL];
extern __IO float OE_LR_Average;
extern __IO float OE_UD_Average;
extern __IO float OE_Focus_Average;
extern __IO float OE_Zoom_Average;
extern _Bool OE_ave_set;
extern uint8_t OE_Shift_Type;
extern uint8_t OE_lmt;
extern uint32_t ADC_ConvertedValue[ADC_NUMOFCHANNEL];
void OE_Shift_calculate(void);
void OE_Shift_AVE(void);
void OE_Shift_limit(void);
/* USER CODE END Private defines */

void MX_ADC1_Init(void);
void Get_Temperature(void);
/* USER CODE BEGIN Prototypes */

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __ADC_H__ */

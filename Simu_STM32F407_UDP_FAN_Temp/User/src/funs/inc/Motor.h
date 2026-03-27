/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : Motor.h
  * @brief          : Header for Motor.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/


/* Includes ------------------------------------------------------------------*/


/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#define IR_Rep3 2
#define IR_Rep2 1
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

#if (Projector_Model == LP92SLB || Projector_Model == NP72BN)
	#define UD_MOTOR_TIM &htim9
	#define LR_MOTOR_TIM &htim9
	#define UD_MOTOR_TIM_CHANNEL TIM_CHANNEL_1
	#define LR_MOTOR_TIM_CHANNEL TIM_CHANNEL_2

#else
	#define UD_MOTOR_TIM &htim4				   // htim4
	#define LR_MOTOR_TIM &htim3				   // htim3
	#define UD_MOTOR_TIM_CHANNEL TIM_CHANNEL_3 // channel 3
	#define LR_MOTOR_TIM_CHANNEL TIM_CHANNEL_4 // channel 4
#endif

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */
extern uint8_t  OE_Shift_dir;
extern uint8_t  OE_Focus_dir;
extern uint8_t  OE_Zoom_dir;
extern _Bool  OE_LR_Reset;
extern _Bool  OE_UD_Reset;
/* USER CODE END EM */



/* Exported functions prototypes ---------------------------------------------*/
extern void OE_Zoom_stop(void);
extern void OE_Focus_stop(void);
extern void OE_ShiftLR_stop(void);
extern void OE_ShiftUD_stop(void);
extern void OE_Shift_Handler(void);
extern void OE_ShiftLR_Enable(void);
extern void OE_ShiftUD_Enable(void);
extern void OE_Zoom_Enable(void);
extern void OE_Focus_Enable(void);

extern void OE_Shift_Reset(void);
extern void OE_Value_clean(void);
/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/

/* USER CODE BEGIN Private defines */
extern uint8_t  motor_dir;
extern _Bool  motor_nsleep;
extern uint8_t  motor_Shift;
extern uint8_t  motor_FZ;
/* USER CODE END Private defines */




/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

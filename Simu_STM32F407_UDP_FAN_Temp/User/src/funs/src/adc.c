/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    adc.c
  * @brief   This file provides code for the configuration
  *          of the ADC instances.
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
#include "adc.h"
#include "error.h"
/* USER CODE BEGIN 0 */
#include "tim.h"
#include "motor.h"
#include <stdlib.h>
#include <math.h>
#include "main.h"

#define average 5
__IO float ADC_ConvertedValueLocal[ADC_NUMOFCHANNEL];
float OE_Shift_LR_AVE[ADC_NUMOFCHANNEL];
float OE_Shift_UD_AVE[ADC_NUMOFCHANNEL];
float OE_Focus_LR_AVE[ADC_NUMOFCHANNEL];
float OE_Zoom_LR_AVE[ADC_NUMOFCHANNEL];
__IO float OE_LR_Average;
__IO float OE_UD_Average;
__IO float OE_Focus_Average;
__IO float OE_Zoom_Average;
_Bool OE_ave_set=0;
uint8_t OE_count=average;
uint8_t OE_lmt;	
uint8_t OE_Shift_Type;
uint32_t ADC_ConvertedValue[ADC_NUMOFCHANNEL];

TempCtl temp_ctl = {
  .dbg_switch = 0 ,
};

const float      Rp=10000;//R是热敏电阻在T2常温下的标称阻值，10K的热敏电阻25℃的值为10K
const float      T2=273.15+25;//常温下的K氏温度
const float      Bx=3975;//热敏电阻的B值参考值
const float      Ka=273.15;//绝对零度开尔文温度
const uint32_t   ADC_pull_up_Res = 4700 ;  //分压电阻阻值
const float      Calibration_parameter = 0.1 ;  //校准参数

/* USER CODE END 0 */

ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

/* ADC1 init function */
void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */
  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = ENABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = ADC_NUMOFCHANNEL;
  hadc1.Init.DMAContinuousRequests = ENABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }
  sConfig.SamplingTime = ADC_SAMPLETIME_480CYCLES;
  sConfig.Offset=0;
#if(Projector_Model == LP92SLB || Projector_Model == NP72BN)
  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_9; // TEMP_AMB
  sConfig.Rank = 1;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_4; //vertical_en
  sConfig.Rank = 2;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_5; //horizontal_en
  sConfig.Rank = 3;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_3;//TEMP_R1
  sConfig.Rank = 4;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_10;//TEMP_R2
  sConfig.Rank = 5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_12;//TEMP_G1
  sConfig.Rank = 6;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
    /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_13;//TEMP_G2
  sConfig.Rank = 7;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
    /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_0;//TEMP_B1 
  sConfig.Rank = 8;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
    /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_8;//TEMP_B2
  sConfig.Rank = 9;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
#else
  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_3;
  sConfig.Rank = 1;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_4;
  sConfig.Rank = 2;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_5;
  sConfig.Rank = 3;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  
  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_8;
  sConfig.Rank = 4;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_9;
  sConfig.Rank = 5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
#endif
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

void HAL_ADC_MspInit(ADC_HandleTypeDef* adcHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(adcHandle->Instance==ADC1)
  {
  /* USER CODE BEGIN ADC1_MspInit 0 */

  /* USER CODE END ADC1_MspInit 0 */
    /* ADC1 clock enable */
    __HAL_RCC_ADC1_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

  #if(Projector_Model == LP92SLB|| Projector_Model == NP72BN)
      /**ADC1 GPIO Configuration
    PA0     ------> ADC1_IN0 TEMP_B1_Pin
    PA3     ------> ADC1_IN3 TEMP_R1_Pin
    PA4     ------> ADC1_IN4 VERTICAL_EN_Pin
    PA5     ------> ADC1_IN5 HORIZONTAL_EN_Pin
    PB0     ------> ADC1_IN8 TEMP_B2_Pin
    PB1     ------> ADC1_IN9 TEMP_AMB_Pin
    PC0     ------> ADC1_IN10 TEMP_R2_Pin
    PC2     ------> ADC1_IN12 TEMP_G1_Pin
    PC3     ------> ADC1_IN13 TEMP_G2_Pin
    */
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    
    GPIO_InitStruct.Pin = TEMP_R1_Pin|TEMP_B1_Pin|VERTICAL_EN_Pin|HORIZONTAL_EN_Pin;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = TEMP_B2_Pin|TEMP_AMB_Pin;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = TEMP_R2_Pin|TEMP_G1_Pin|TEMP_G2_Pin;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  #else
    /**ADC1 GPIO Configuration
    PA3     ------> ADC1_IN3 TEC_CFB_Pin
    PA4     ------> ADC1_IN4 VERTICAL_EN_Pin
    PA5     ------> ADC1_IN5 HORIZONTAL_EN_Pin
    PB0     ------> ADC1_IN8 ZOOM_EN_Pin
    PB1     ------> ADC1_IN9 FOCUS_EN_Pin
    */
    GPIO_InitStruct.Pin = TEC_CFB_Pin|VERTICAL_EN_Pin|HORIZONTAL_EN_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  #endif 

    /* ADC1 DMA Init */
    /* ADC1 Init */
    hdma_adc1.Instance = DMA2_Stream0;
    hdma_adc1.Init.Channel = DMA_CHANNEL_0;
    hdma_adc1.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_adc1.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_adc1.Init.MemInc = DMA_MINC_ENABLE;
    hdma_adc1.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    hdma_adc1.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
    hdma_adc1.Init.Mode = DMA_CIRCULAR;
    hdma_adc1.Init.Priority = DMA_PRIORITY_VERY_HIGH;
    hdma_adc1.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_adc1) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(adcHandle,DMA_Handle,hdma_adc1);

  /* USER CODE BEGIN ADC1_MspInit 1 */
    HAL_NVIC_SetPriority(ADC_IRQn, 1, 2);
    HAL_NVIC_EnableIRQ(ADC_IRQn);
  /* USER CODE END ADC1_MspInit 1 */
  }
}

void HAL_ADC_MspDeInit(ADC_HandleTypeDef* adcHandle)
{

  if(adcHandle->Instance==ADC1)
  {
  /* USER CODE BEGIN ADC1_MspDeInit 0 */

  /* USER CODE END ADC1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_ADC1_CLK_DISABLE();

    /**ADC1 GPIO Configuration
    PA3     ------> ADC1_IN3
    PA4     ------> ADC1_IN4
    PA5     ------> ADC1_IN5
    PB0     ------> ADC1_IN8
    PB1     ------> ADC1_IN9
    */
#if(Projector_Model == LP92SLB || Projector_Model == NP72BN)
    HAL_GPIO_DeInit(GPIOA, TEMP_R1_Pin|TEMP_B1_Pin|VERTICAL_EN_Pin|HORIZONTAL_EN_Pin);
    HAL_GPIO_DeInit(GPIOB, TEMP_B2_Pin|TEMP_AMB_Pin);
    HAL_GPIO_DeInit(GPIOC, TEMP_R2_Pin|TEMP_G1_Pin|TEMP_G2_Pin);
#else
    HAL_GPIO_DeInit(GPIOA, TEC_CFB_Pin|VERTICAL_EN_Pin|HORIZONTAL_EN_Pin);
#endif
//    HAL_GPIO_DeInit(GPIOB, ZOOM_EN_Pin|FOCUS_EN_Pin);

    /* ADC1 DMA DeInit */
    HAL_DMA_DeInit(adcHandle->DMA_Handle);
  /* USER CODE BEGIN ADC1_MspDeInit 1 */
    HAL_NVIC_DisableIRQ(ADC_IRQn);
  /* USER CODE END ADC1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
}


void HAL_ADC_ErrorCallback(ADC_HandleTypeDef *hadc)
{
	 printf("HAL_ADC_ErrorCallback\r\n");
}


/*******************************
垂直方向,由上到下,VERTICAL_EN 由大变小 ,从2.444V变为0.845V
水平方向，由左到右，HORIZONTAL_EN 由小变大，从2.125V变为 1.121V 
****************************/
void OE_Shift_calculate ( void )
{
    /**ADC1 GPIO Configuration
    PA3     ------> ADC1_IN3  TEC_CFB_Pin
    PA4     ------> ADC1_IN4 VERTICAL_EN_Pin
    PA5     ------> ADC1_IN5 HORIZONTAL_EN_Pin
    PB0     ------> ADC1_IN8 ZOOM_EN_Pin
    PB1     ------> ADC1_IN9 FOCUS_EN_Pin
    */
	
    // ADC_ConvertedValueLocal[0] = ( float ) ( ADC_ConvertedValue[0]&0xFFF ) *3.3f/4096; // ADC_ConvertedValue[0]只取最低12有效数据
    ADC_ConvertedValueLocal[1] = ( float ) ( ADC_ConvertedValue[1]&0xFFF ) *3.3f/4096;
    ADC_ConvertedValueLocal[2] = ( float ) ( ADC_ConvertedValue[2]&0xFFF ) *3.3f/4096;
//    ADC_ConvertedValueLocal[3] = ( float ) ( ADC_ConvertedValue[3]&0xFFF ) *3.3f/4096;
//    ADC_ConvertedValueLocal[4] = ( float ) ( ADC_ConvertedValue[4]&0xFFF ) *3.3f/4096;

//    printf(" TEC_CFB_Pin: %.3fV\r\n", ADC_ConvertedValueLocal[0]);
//    printf(" VERTICAL_EN_Pin: %.3fV\r\n", ADC_ConvertedValueLocal[1]);
//    printf(" HORIZONTAL_EN_Pin: %.3fV\r\n", ADC_ConvertedValueLocal[2]);
//	  printf(" ZOOM_EN_Pin: %.3fV\r\n", ADC_ConvertedValueLocal[3]);
//    printf(" FOCUS_EN_Pin: %.3fV\r\n", ADC_ConvertedValueLocal[4]);


}
void OE_Shift_AVE ( void )
{
	if(OE_count)
	{
		OE_Shift_UD_AVE[OE_count-1]=ADC_ConvertedValueLocal[1];
		OE_Shift_LR_AVE[OE_count-1]=ADC_ConvertedValueLocal[2];
//		OE_Zoom_LR_AVE[OE_count-1]=ADC_ConvertedValueLocal[3];
//		OE_Focus_LR_AVE[OE_count-1]=ADC_ConvertedValueLocal[4];	
		OE_ave_set=0;
		if(--OE_count==0)
		{
			OE_UD_Average=(OE_Shift_UD_AVE[0]+OE_Shift_UD_AVE[1]+OE_Shift_UD_AVE[2]+OE_Shift_UD_AVE[3]+OE_Shift_UD_AVE[4])/average;
			OE_LR_Average=(OE_Shift_LR_AVE[0]+OE_Shift_LR_AVE[1]+OE_Shift_LR_AVE[2]+OE_Shift_LR_AVE[3]+OE_Shift_LR_AVE[4])/average;
//			OE_Zoom_Average=(OE_Zoom_LR_AVE[0]+OE_Zoom_LR_AVE[1]+OE_Zoom_LR_AVE[2]+OE_Zoom_LR_AVE[3]+OE_Zoom_LR_AVE[4])/average;
//			OE_Focus_Average=(OE_Focus_LR_AVE[0]+OE_Focus_LR_AVE[1]+OE_Focus_LR_AVE[2]+OE_Focus_LR_AVE[3]+OE_Focus_LR_AVE[4])/average;
			OE_ave_set=1;
			OE_count=average;
			
//		  printf(" VERTICAL_EN: %.3fV\r\n", OE_UD_Average);
//			printf(" HORIZONTAL_EN: %.3fV\r\n", OE_LR_Average);
//			printf(" ZOOM_EN: %.3fV\r\n", OE_Zoom_Average);
//			printf(" FOCUS_EN: %.3fV\r\n", OE_Focus_Average);
		}
	}
}

void OE_Shift_limit ( void )
{
	if(OE_ave_set)
	{
	
		switch (OE_Shift_Type)
		{
			case OE_Shift_LR  :  
			{
				printf(" HORIZONTAL_EN: %.3fV\r\n", OE_LR_Average);
//				printf(" OE_Shift_L: %.3fV\r\n", OE_Shift_L);
//				printf(" OE_Shift_R: %.3fV\r\n", OE_Shift_R);
				if(OE_LR_Average>OE_Shift_L&&OE_Shift_dir==0x22)
				{
           HAL_GPIO_WritePin(MOTOR3_ENABLE_GPIO_Port, MOTOR3_ENABLE_Pin, GPIO_PIN_SET);
	         HAL_TIM_PWM_Stop( LR_MOTOR_TIM, LR_MOTOR_TIM_CHANNEL );
	         HAL_GPIO_WritePin(MOTOR2_RESET_GPIO_Port, MOTOR2_RESET_Pin, GPIO_PIN_RESET);
					 OE_lmt=0x11;
					printf("Shift_L_Out\r\n");
				}
				else if(OE_LR_Average<OE_Shift_R&&OE_Shift_dir==0x11)
				{
           HAL_GPIO_WritePin(MOTOR3_ENABLE_GPIO_Port, MOTOR3_ENABLE_Pin, GPIO_PIN_SET);
	         HAL_TIM_PWM_Stop( LR_MOTOR_TIM, LR_MOTOR_TIM_CHANNEL );
	         HAL_GPIO_WritePin(MOTOR2_RESET_GPIO_Port, MOTOR2_RESET_Pin, GPIO_PIN_RESET);	
           OE_lmt=0x22;					
					printf("Shift_R_Out\r\n");
				}
				else
					OE_lmt=0;
			}
			 break;	
			case  OE_Shift_UD:  
			{
				 printf(" VERTICAL_EN: %.3fV\r\n", OE_UD_Average);
//				printf(" OE_Shift_U: %.3fV\r\n", OE_Shift_U);
//				printf(" OE_Shift_D: %.3fV\r\n", OE_Shift_D);				
				if(OE_UD_Average>OE_Shift_U&&OE_Shift_dir==0x22)
				{
					 HAL_GPIO_WritePin(MOTOR2_ENABLE_GPIO_Port, MOTOR2_ENABLE_Pin, GPIO_PIN_SET);
					HAL_TIM_PWM_Stop ( UD_MOTOR_TIM, UD_MOTOR_TIM_CHANNEL );
					HAL_GPIO_WritePin(MOTOR2_RESET_GPIO_Port, MOTOR2_RESET_Pin, GPIO_PIN_RESET);
					OE_lmt=0x33;	
					 printf("Shift_U_Out\r\n");
				}
				else if(OE_UD_Average<OE_Shift_D&&OE_Shift_dir==0x11)
				{
					HAL_GPIO_WritePin(MOTOR2_ENABLE_GPIO_Port, MOTOR2_ENABLE_Pin, GPIO_PIN_SET);
					HAL_TIM_PWM_Stop (UD_MOTOR_TIM, UD_MOTOR_TIM_CHANNEL );
					HAL_GPIO_WritePin(MOTOR2_RESET_GPIO_Port, MOTOR2_RESET_Pin, GPIO_PIN_RESET);
					OE_lmt=0x44;	
					 printf("Shift_D_Out\r\n");					
				}
				else 
					OE_lmt=0;
			}
			 break;	
//			case OE_Zoom:
//			{
//				if(OE_Zoom_Average>OE_zoom_L||OE_Zoom_Average<OE_zoom_R)
//				{
//					OE_Zoom_stop();
//					OE_Shift_Type=0;
//					OE_Shift_dir=0;
//					OE_movement=0;					
//					printf("Zoom_Out\r\n");
//				}
//			}
//			 break;					
//			case OE_Focus:
//			{
//				if(OE_Focus_Average>OE_focus_L||OE_Focus_Average<OE_focus_R)
//			  {
//					OE_Focus_stop();
//					OE_Shift_Type=0;
//					OE_Shift_dir=0;
//					OE_movement=0;					
//					printf("Focus_Out\r\n");
//				}
//			}
//			 break;			
      default: break;			
		}
	}
	
}

/**
 * @brief 热敏电阻温度计算函数
 * 
 * @param i 
 * @return float 
 */
float NTC_temp_calc ( float adc_vol )
{
    float Rt=0;//Rt 热敏电阻在T1温度下的阻值
    float temp=0;//测量得到的温度
    Rt = adc_vol*ADC_pull_up_Res/ ( 3.3f - adc_vol ); //ADC采样电压结果
    temp=1/ ( 1/T2+ (log ( Rt/Rp )) /Bx )-Ka + Calibration_parameter; //温度值反计算，计算公式详见NTC数据手册
    if(temp < -19 ) //超出传感器有效范围，无意义，限制为-19℃
      temp = -19 ;
    if(temp > 125) //超出传感器有效范围，无意义，限制为125℃
      temp = 125 ;
    return temp;
}

/**
 * @brief 温度限幅滤波器，用于滤除温度检测异常值
 * 
 * @param new_temp  新计算出的温度
 * @param last_temp 上一次记录的正常温度,用于比较,若新温度有效，则更新该值
 * @param clip_lmt 温度变化阈值 
 * @return float 本次有效温度值
 */
float temp_clip_filter(unsigned char i, float new_temp, float *last_temp, float clip_lmt)
{
  static unsigned char err_cnt[7] = {0} ;

  if(abs(new_temp - *last_temp) < clip_lmt) //温度正常变化
  {
    *last_temp = new_temp;
    err_cnt[i] = 0 ;
    return new_temp;
  }
  else
  {
    if(err_cnt[i] < 2) //温度异常变化，但是连续两次都是异常变化，才认为是真的异常
    {
      err_cnt[i]++;
      return *last_temp;
    }
    else
    {
      *last_temp = new_temp;
      err_cnt[i] = 0 ;
      return new_temp;
    }
  }
}

/**
 * @brief 温度采集函数
 * 
 */
void Get_Temperature( void )
{
  static unsigned char first_flag = 1;
  static float last_temp[7] = {-255,-255,-255,-255,-255,-255,-255};
  const static int DELTA_TEMP_MAX = 20;  //℃
  static float temperature[7] = {0};  //运算用温度暂存数组

#if(Projector_Model == LP92SLB || Projector_Model == NP72BN)
  //电压采集
  ADC_ConvertedValueLocal[0] = ( float ) ( ADC_ConvertedValue[0] & 0xFFF) *3.3f/4096;
  // ADC_ConvertedValueLocal[1] = ( float ) ( ADC_ConvertedValue[1] & 0xFFF) *3.3/4096;
  // ADC_ConvertedValueLocal[2] = ( float ) ( ADC_ConvertedValue[2] & 0xFFF) *3.3/4096;
  ADC_ConvertedValueLocal[3] = ( float ) ( ADC_ConvertedValue[3] & 0xFFF) *3.3f/4096;
  ADC_ConvertedValueLocal[4] = ( float ) ( ADC_ConvertedValue[4] & 0xFFF) *3.3f/4096;
  ADC_ConvertedValueLocal[5] = ( float ) ( ADC_ConvertedValue[5] & 0xFFF) *3.3f/4096;
  ADC_ConvertedValueLocal[6] = ( float ) ( ADC_ConvertedValue[6] & 0xFFF) *3.3f/4096;
  ADC_ConvertedValueLocal[7] = ( float ) ( ADC_ConvertedValue[7] & 0xFFF) *3.3f/4096;
  ADC_ConvertedValueLocal[8] = ( float ) ( ADC_ConvertedValue[8] & 0xFFF) *3.3f/4096;

  //NTC温度计算
  temperature[0]=NTC_temp_calc(ADC_ConvertedValueLocal[3]); // TEMP_R1
	temperature[1]=NTC_temp_calc(ADC_ConvertedValueLocal[4]); // TEMP_R2
	temperature[2]=NTC_temp_calc(ADC_ConvertedValueLocal[5]); // TEMP_G1
	temperature[3]=NTC_temp_calc(ADC_ConvertedValueLocal[6]); // TEMP_G2
	temperature[4]=NTC_temp_calc(ADC_ConvertedValueLocal[7]); // TEMP_B1
	temperature[5]=NTC_temp_calc(ADC_ConvertedValueLocal[8]); // TEMP_B2
	temperature[6]=NTC_temp_calc(ADC_ConvertedValueLocal[0]); // TEMP_AMB/ENV

#else
  temperature[0] = ADS1115_convert(TI_ADS1115_CH0, ADS1115_I2C_Address); //R1
  temperature[1] = ADS1115_convert(TI_ADS1115_CH1, ADS1115_I2C_Address); //R2
  temperature[2] = ADS1115_convert(TI_ADS1115_CH2, ADS1115_I2C_Address); //G1
  temperature[3] = ADS1115_convert(TI_ADS1115_CH3, ADS1115_I2C_Address); //G2
  temperature[4] = ADS1115_convert(TI_ADS1115_CH0, ADS1015_I2C_Address); //B1
#if Temp_B2
  temperature[5] = ADS1115_convert(TI_ADS1115_CH2, ADS1015_I2C_Address); //B2
#endif
  temperature[6] = ADS1115_convert(TI_ADS1115_CH3, ADS1015_I2C_Address); //ENV
  // temperature[5] = ADS1115_convert(TI_ADS1115_CH1, ADS1015_I2C_Address); //TEC
#endif

  //filter
  if(first_flag) //第一次温度采集时直接复制值为上一次值
  {
    for (unsigned char i = 0; i < 7; i++)
    {
      last_temp[i] = temperature[i];
    }
    first_flag = 0;
  }
  else //除了第一次，温度都需要进行限幅滤波
  {
    for (unsigned char i = 0; i < 7; i++)
    {
      temperature[i] = temp_clip_filter(i,temperature[i], &last_temp[i], DELTA_TEMP_MAX);
    }
  }

  //温度数据存储，用于发送给Mstar  
  ADS_Temp_Data[0] = temperature[0];
  ADS_Temp_Data[1] = temperature[1];
  ADS_Temp_Data[2] = temperature[2];
  ADS_Temp_Data[3] = temperature[3];
  ADS_Temp_Data[4] = temperature[4];
  ADS_Temp_Data[6] = temperature[5];  // TEMP_B2
  ADS_Temp_Data[7] = temperature[6];
  

  //温度测试接口，用于调试，制造错误数据传给Mstar
  if(temp_ctl.dbg_switch)
    ADS_Temp_Data[temp_ctl.dbg_temp_idx] = temp_ctl.dbg_temp_val;
  
  printf(" \r\n");
  printf(" TEMP_R_LD1: %.2f degC\r\n", ADS_Temp_Data[0]);
  printf(" TEMP_R_LD2: %.2f degC\r\n", ADS_Temp_Data[1]);
  printf(" TEMP_G_LD1: %.2f degC\r\n", ADS_Temp_Data[2]);
  printf(" TEMP_G_LD2: %.2f degC\r\n", ADS_Temp_Data[3]);
  printf(" TEMP_B_LD1: %.2f degC\r\n", ADS_Temp_Data[4]);
#if Temp_B2
  printf(" TEMP_B_LD2: %.2f degC\r\n", ADS_Temp_Data[6]);
#endif
  printf(" TEMP_Environment : %.2f degC\r\n", ADS_Temp_Data[7]);
  
}

/* USER CODE END 1 */

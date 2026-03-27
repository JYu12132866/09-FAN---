/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    i2c.c
 * @brief   This file provides code for the configuration
 *          of the I2C instances.
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
#include "i2c.h"

/* USER CODE BEGIN 0 */
#include "tim.h"
#include "error.h"
#include "motor.h"
#include "stmflash.h"
#include "main.h"
#include "udp_demo.h"
#include "lan8720.h"
#include "c381.h"
#include "modectl.h"
#include "rt_param.h"
#include "healthMgr.h"
#include "pjlink.h"
#include "logger.h"
#include "power_monitor.h"

#if (Projector_Model == LP92SLB || Projector_Model == NP72BN)
#include "it6635_import.h"
#endif

//函数前向声明, 避免编译警告
void it6635_dev_restart(void);


#define I2C1_Slave_Addr 0x32

extern uint8_t mstar_work_on_oneshot_flag;

uint8_t DMD_Temp;
uint8_t Worktime[2];
extern uint8_t i2cRxBuffer[2];
extern uint8_t FAN_Temp_Error[7];
// extern uint8_t timer;
extern uint8_t FrameNo;
_Bool I2C_IT_Trigger = FALSE;
HAL_StatusTypeDef I2C_status = HAL_OK;
HAL_StatusTypeDef I2C_OLD_Sat = HAL_OK;
/* USER CODE END 0 */
// _Bool I2C_Read_in = FALSE;
I2C_HandleTypeDef hi2c1;
I2C_HandleTypeDef hi2c2;
uint8_t I2C_Busy2;
uint8_t I2C_Busy3;
uint8_t I2C_Busy1;
/* I2C1 init function */
void MX_I2C1_Init(void)
{

	/* USER CODE BEGIN I2C1_Init 0 */
	//
	/* USER CODE END I2C1_Init 0 */

	/* USER CODE BEGIN I2C1_Init 1 */

	/* USER CODE END I2C1_Init 1 */
	hi2c1.Instance = I2C1;
	hi2c1.Init.ClockSpeed = 100000;
	hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
	hi2c1.Init.OwnAddress1 = I2C1_Slave_Addr;
	hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
	hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	hi2c1.Init.OwnAddress2 = 0;
	hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
	if (HAL_I2C_Init(&hi2c1) != HAL_OK)
	{
		Error_Handler();
	}
	/* USER CODE BEGIN I2C1_Init 2 */

	/* USER CODE END I2C1_Init 2 */
}
/* I2C2 init function */
void MX_I2C2_Init(void)
{

	/* USER CODE BEGIN I2C2_Init 0 */

	/* USER CODE END I2C2_Init 0 */

	/* USER CODE BEGIN I2C2_Init 1 */

	/* USER CODE END I2C2_Init 1 */
	hi2c2.Instance = I2C2;
	hi2c2.Init.ClockSpeed = 100000;
	hi2c2.Init.DutyCycle = I2C_DUTYCYCLE_16_9;
	hi2c2.Init.OwnAddress1 = 0;
	hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
	hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	hi2c2.Init.OwnAddress2 = 0;
	hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
	if (HAL_I2C_Init(&hi2c2) != HAL_OK)
	{
		Error_Handler();
	}
	/* USER CODE BEGIN I2C2_Init 2 */

	/* USER CODE END I2C2_Init 2 */
}

void HAL_I2C_MspInit(I2C_HandleTypeDef *i2cHandle)
{

	GPIO_InitTypeDef GPIO_InitStruct = {0};
	if (i2cHandle->Instance == I2C1)
	{
		/* USER CODE BEGIN I2C1_MspInit 0 */
		__HAL_RCC_GPIOB_CLK_ENABLE();
		GPIO_InitStruct.Pin = MCU0_I2C1_SCL_Pin | MCU0_I2C1_SDA_Pin;
		GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
		GPIO_InitStruct.Pull = GPIO_PULLUP;
		GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
		HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
		HAL_GPIO_WritePin(GPIOB, MCU0_I2C1_SCL_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, MCU0_I2C1_SDA_Pin, GPIO_PIN_SET);
		//		 __HAL_RCC_I2C1_FORCE_RESET();
		//		 __HAL_RCC_I2C1_RELEASE_RESET();
		hi2c1.Instance->CR1 |= I2C_CR1_SWRST; // 20220610
		hi2c1.Instance->CR1 &= ~I2C_CR1_SWRST;
		/* USER CODE END I2C1_MspInit 0 */

		/**I2C1 GPIO Configuration
		PB6     ------> I2C1_SCL
		PB7     ------> I2C1_SDA
		*/
		GPIO_InitStruct.Pin = MCU0_I2C1_SCL_Pin | MCU0_I2C1_SDA_Pin;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
		GPIO_InitStruct.Pull = GPIO_PULLUP;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
		HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

		/* I2C1 clock enable */
		__HAL_RCC_I2C1_CLK_ENABLE();

		/* I2C1 interrupt Init */
		HAL_NVIC_SetPriority(I2C1_EV_IRQn, 1, 1);
		HAL_NVIC_EnableIRQ(I2C1_EV_IRQn);
		HAL_NVIC_SetPriority(I2C1_ER_IRQn, 1, 1);
		HAL_NVIC_EnableIRQ(I2C1_ER_IRQn);
		/* USER CODE BEGIN I2C1_MspInit 1 */

		/* USER CODE END I2C1_MspInit 1 */
	}
	else if (i2cHandle->Instance == I2C2)
	{
		/* USER CODE BEGIN I2C2_MspInit 0 */
		GPIO_InitStruct.Pin = TMP_SDA_Pin | TMP_SDA_Pin;
		GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
		GPIO_InitStruct.Pull = GPIO_PULLUP;
		GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
		HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);
		HAL_GPIO_WritePin(GPIOF, TMP_SDA_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOF, TMP_SCL_Pin, GPIO_PIN_SET);
		i2cHandle->Instance->CR1 = I2C_CR1_SWRST;
		i2cHandle->Instance->CR1 = 0;
		/* USER CODE END I2C2_MspInit 0 */

		__HAL_RCC_GPIOF_CLK_ENABLE();
		/**I2C2 GPIO Configuration
		PF0     ------> I2C2_SDA
		PF1     ------> I2C2_SCL
		*/
		GPIO_InitStruct.Pin = TMP_SDA_Pin | TMP_SCL_Pin;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF4_I2C2;
		HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

		/* I2C2 clock enable */
		__HAL_RCC_I2C2_CLK_ENABLE();

		/* I2C2 interrupt Init */
		HAL_NVIC_SetPriority(I2C2_EV_IRQn, 2, 0);
		HAL_NVIC_EnableIRQ(I2C2_EV_IRQn);
		HAL_NVIC_SetPriority(I2C2_ER_IRQn, 2, 1);
		HAL_NVIC_EnableIRQ(I2C2_ER_IRQn);
		/* USER CODE BEGIN I2C2_MspInit 1 */

		/* USER CODE END I2C2_MspInit 1 */
	}
}

void HAL_I2C_MspDeInit(I2C_HandleTypeDef *i2cHandle)
{

	if (i2cHandle->Instance == I2C1)
	{
		/* USER CODE BEGIN I2C1_MspDeInit 0 */

		/* USER CODE END I2C1_MspDeInit 0 */
		/* Peripheral clock disable */
		__HAL_RCC_I2C1_CLK_DISABLE();

		/**I2C1 GPIO Configuration
		PB6     ------> I2C1_SCL
		PB7     ------> I2C1_SDA
		*/
		HAL_GPIO_DeInit(MCU0_I2C1_SCL_GPIO_Port, MCU0_I2C1_SCL_Pin);

		HAL_GPIO_DeInit(MCU0_I2C1_SDA_GPIO_Port, MCU0_I2C1_SDA_Pin);

		/* I2C1 interrupt Deinit */
		HAL_NVIC_DisableIRQ(I2C1_EV_IRQn);
		HAL_NVIC_DisableIRQ(I2C1_ER_IRQn);
		/* USER CODE BEGIN I2C1_MspDeInit 1 */

		/* USER CODE END I2C1_MspDeInit 1 */
	}
	else if (i2cHandle->Instance == I2C2)
	{
		/* USER CODE BEGIN I2C2_MspDeInit 0 */

		/* USER CODE END I2C2_MspDeInit 0 */
		/* Peripheral clock disable */
		__HAL_RCC_I2C2_CLK_DISABLE();

		/**I2C2 GPIO Configuration
		PF0     ------> I2C2_SDA
		PF1     ------> I2C2_SCL
		*/
		HAL_GPIO_DeInit(TMP_SDA_GPIO_Port, TMP_SDA_Pin);

		HAL_GPIO_DeInit(TMP_SCL_GPIO_Port, TMP_SCL_Pin);

		/* I2C2 interrupt Deinit */
		HAL_NVIC_DisableIRQ(I2C2_EV_IRQn);
		HAL_NVIC_DisableIRQ(I2C2_ER_IRQn);
		/* USER CODE BEGIN I2C2_MspDeInit 1 */

		/* USER CODE END I2C2_MspDeInit 1 */
	}
}
// int8_t test=-19;
// int8_t test2=-1;
// int8_t test3=80;
/* USER CODE BEGIN 1 */

void HAL_I2C_SlaveTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
#if 0 
	uint8_t Tempture_Error[12];
	// uint16_t timeout=0;

	if ((int8_t)ADS_Temp_Data[0] < 0)
		Tempture_Error[0] = (int8_t)ADS_Temp_Data[0] - 1; // TMP1_R_LD1
	else
		Tempture_Error[0] = (int8_t)ADS_Temp_Data[0]; // TMP1_R_LD1

	if ((int8_t)ADS_Temp_Data[1] < 0)
		Tempture_Error[1] = (int8_t)ADS_Temp_Data[1] - 1; // TMP1_R_LD2
	else
		Tempture_Error[1] = (int8_t)ADS_Temp_Data[1];

	if ((int8_t)ADS_Temp_Data[2] < 0)
		Tempture_Error[2] = (int8_t)ADS_Temp_Data[2] - 1; // TMP1_G_LD1
	else
		Tempture_Error[2] = (int8_t)ADS_Temp_Data[2];

	if ((int8_t)ADS_Temp_Data[3] < 0)
		Tempture_Error[3] = (int8_t)ADS_Temp_Data[3] - 1; // TMP1_G_LD2
	else
		Tempture_Error[3] = (int8_t)ADS_Temp_Data[3];

	if ((int8_t)ADS_Temp_Data[4] < 0)
		Tempture_Error[4] = (int8_t)ADS_Temp_Data[4] - 1; // TMP2_B_LD1
	else
		Tempture_Error[4] = (int8_t)ADS_Temp_Data[4]; // TMP2_B_LD1

	if ((int8_t)ADS_Temp_Data[5] < 0)
		Tempture_Error[5] = (int8_t)ADS_Temp_Data[5] - 1; // TMP2_TEC
	else
		Tempture_Error[5] = (int8_t)ADS_Temp_Data[5]; // TMP2_TEC

	if ((int8_t)ADS_Temp_Data[7] < 0)
		Tempture_Error[6] = (int8_t)ADS_Temp_Data[7] - 1; // TMP2_Environment
	else
		Tempture_Error[6] = (int8_t)ADS_Temp_Data[7]; // TMP2_Environment
#if Temp_B2
	if ((int8_t)ADS_Temp_Data[6] < 0)
		Tempture_Error[9] = (int8_t)ADS_Temp_Data[6] - 1; // TMP2_B_LD2
	else
		Tempture_Error[9] = (int8_t)ADS_Temp_Data[6];
#endif

	Tempture_Error[7] = FAN_Warning[0]; // FAN_1_Speed
	Tempture_Error[8] = FAN_Warning[1]; // FAN_2_Speed

	if (hi2c->Instance == I2C1)
	{
		switch (I2C_CMD) // 20220704
		{
		case 0x40: //
		{
			HAL_I2C_Slave_Transmit(&hi2c1, &Tempture_Error[0], 1, IIC_Timeout); // TMP1_R_LD1
		}
		break;
		case 0x41: //
		{
			HAL_I2C_Slave_Transmit(&hi2c1, &Tempture_Error[1], 1, IIC_Timeout); ////TMP1_R_LD2
		}
		break;
		case 0x42: //
		{
			HAL_I2C_Slave_Transmit(&hi2c1, &Tempture_Error[2], 1, IIC_Timeout); ////TMP1_G_LD1
		}
		break;
		case 0x43: //
		{
			HAL_I2C_Slave_Transmit(&hi2c1, &Tempture_Error[3], 1, IIC_Timeout); // TMP1_G_LD2
		}
		break;
		case 0x44: //
		{
			HAL_I2C_Slave_Transmit(&hi2c1, &Tempture_Error[4], 1, IIC_Timeout); // TMP2_B_LD1
		}
		break;
		case 0x45: //
		{
			HAL_I2C_Slave_Transmit(&hi2c1, &Tempture_Error[9], 1, IIC_Timeout); // TMP2_B_LD2
		}
		break;
		case 0x46: //
		{
			HAL_I2C_Slave_Transmit(&hi2c1, &Tempture_Error[6], 1, IIC_Timeout); // TMP2_Environment
		}
		break;
		case 0x47: //
		{
			HAL_I2C_Slave_Transmit(&hi2c1, &Tempture_Error[7], 1, IIC_Timeout); // FAN_1_Speed
		}
		break;
		case 0x48: //
		{
			HAL_I2C_Slave_Transmit(&hi2c1, &Tempture_Error[8], 1, IIC_Timeout); // FAN_2_Speed
		}
		break;
		case 0x49: //
		{
			HAL_I2C_Slave_Transmit(&hi2c1, &Tempture_Error[10], 1, IIC_Timeout); // sw version
		}
		break;
		default:
			break;
		}
	}
#endif
}


void Reset_I2C(void)
{
	uint16_t timeout = 0;

	printf("I2C_Busy1=0x%x\r\n", I2C_Busy1);
	printf("I2C_Busy2=0x%x\r\n", I2C_Busy2);
	printf("I2C_Busy3=0x%x\r\n", I2C_Busy3);

	HAL_I2C_DeInit(&hi2c1);
	timeout = 0;
	while (HAL_I2C_GetState(&hi2c1) != HAL_I2C_STATE_RESET)
	{
		timeout++;
		if (timeout > 0xFFFE)
		{
			printf("!HAL_I2C_STATE_RESET\n");
			timeout = 0;
			break;
		}
		HAL_Delay(100);
	}
	MX_I2C1_Init();
	timeout = 0;
	while (HAL_I2C_GetState(&hi2c1) != HAL_I2C_STATE_READY)
	{
		timeout++;
		if (timeout > 0xFFFE)
		{
			printf("!HAL_I2C_STATE_READY\n");
			timeout = 0;

			break;
		}
		HAL_Delay(100);
	}
	timeout = 0;
	while (!(HAL_GPIO_ReadPin(GPIOB, MCU0_I2C1_SCL_Pin)))
	{
		timeout++;
		if (timeout > 0xFFFE)
		{
			printf("!MCU0_I2C1_SCL_Pin\n");
			timeout = 0;

			break;
		}
		HAL_Delay(100);
	}
	timeout = 0;
	//	printf("I2C1_SCL_Pin=%d\r\n",(HAL_GPIO_ReadPin(GPIOB, MCU0_I2C1_SCL_Pin)));
	//	printf("I2C1_SDA_Pin=%d\r\n",(HAL_GPIO_ReadPin(GPIOB, MCU0_I2C1_SDA_Pin)));
	while (HAL_OK != HAL_I2C_Slave_Receive_IT(&hi2c1, (uint8_t *)i2cRxBuffer, 2))
	{
		timeout++;
		if (timeout > 0xFFFE)
		{
			timeout = 0;
			break;
		}
		HAL_Delay(100);
	}
	timeout = 0;

	printf("Reset_I2C\r\n");
}

void HAL_I2C_SlaveRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	I2C_IT_Trigger = TRUE;
}

void I2C_CMD_Process(void)
{
	if (!I2C_IT_Trigger)
		return;

	I2C_IT_Trigger = FALSE;
	uint16_t timeout = 0;
	uint8_t k;
	/******************************************************/
	{
		printf("[I2C_CMD]:0x%x-0x%x\n", i2cRxBuffer[0], i2cRxBuffer[1]);

		switch (i2cRxBuffer[0])
		{
#ifdef C381
		case I2C_CurMode:
		{
			CursorMode = i2cRxBuffer[1];
		}
		break;

		case I2C_CurSel_LR:
		{
			CursorSel_LR = i2cRxBuffer[1];
		}
		break;
		case I2C_CurSel_UD:
		{
			CursorSel_UD = i2cRxBuffer[1];
		}
		break;
		case I2C_Warp_LR:
		{
			Wraping_LR = i2cRxBuffer[1];
		}
		break;
		case I2C_Warp_UD:
		{
			Wraping_UD = i2cRxBuffer[1];
		}
		break;
		case I2C_Warp_Rst:
		{
			Reset_Wrap = i2cRxBuffer[1];
		}
		break;
		case I2C_Warp_Load:
		{
			Load_Wrap = i2cRxBuffer[1];
			WP_VkeystoneStatus = 0;
			WP_HkeystoneStatus = 0;
		}
		break;
		case I2C_Warp_Save:
		{
			Save_Wrap = i2cRxBuffer[1];
			WP_VkeystoneStatus = 0;
			WP_HkeystoneStatus = 0;
		
		}
		break;
		case I2C_C381_Mode:
		{
			// HAL_Delay(1000);
			C381ModeChange = TRUE;
			rt_param.c381_mode = i2cRxBuffer[1];
			save_rt_param_to_flash();
			// video_src_changed = TRUE;
			// it6635_dev_restart();
		}
		case I2C_Fake_Auto_Warp:
		{
			autoWarpDemo = 1 ;
		}
		break;
#endif

		case I2C_WorkMode: // Ethernet IP Address update
		{
			modectl_switch_mode(i2cRxBuffer[1]);
			// printf("I2C_WorkMode=0x%x\r\n",FAN_DC);
		}
		break;
		case I2C_Worktime1: // Ethernet IP Address update
		{
			Worktime[0] = i2cRxBuffer[1];
			printf("I2C_Worktime1=%d\n", Worktime[0]);
		}
		break;
		case I2C_Worktime2: // Ethernet IP Address update
		{
			Worktime[1] = i2cRxBuffer[1];
			printf("I2C_Worktime2=%d\n", Worktime[1]);
		}
		break;

		case I2C_IP_ADJ: // Ethernet IP Address update
		{
			// #if Ethernet
			if (i2cRxBuffer[1] == 20)
			{
				ResetIP();

				printf("IPReset\n");
				STMFLASH_Write(ADDR_FLASH_SECTOR_6, (uint32_t *)Ethernet_Info, 9);
				lwipIPUpdate();
				printf("IPSave\n");
				HAL_UART_Transmit(&huart1, (uint8_t *)"Ucn", 3, 5000);
			}
			// #endif
		}
		break;
		case I2C_Report_Sel: // xml enable
		{
			printf("I2C_Report_Sel=0x%x\r\n", i2cRxBuffer[1]);

			rt_param.health_report_format = i2cRxBuffer[1];
			if (rt_param.health_report_format)
			{
				if (FrameNo != 0)
					FrameNo = 0;
			}
			//  FAN_Temp_Error[5]=Report_Sel;
			save_rt_param_to_flash();
		}
		break;
		case I2C_XML_Enable: // xml enable
		{
			printf("I2C_XML_Enable=0x%x\r\n", i2cRxBuffer[1]);
			rt_param.health_report_en = i2cRxBuffer[1];
			save_rt_param_to_flash();
		}
		break;
		case I2C_DMD_Temp: // DMD Temperature
		{
			printf("DMD_Temp=0x%x\r\n", i2cRxBuffer[1]);
			DMD_Temp = i2cRxBuffer[1];
		}

		break;
		case I2C_Work_Sat: // work status
		{
			printf("work_status=0x%x\r\n", i2cRxBuffer[1]);
			if (i2cRxBuffer[1] == 0x3F) // power off
			{
				Work_Stat = 0;
				power_monitor_on_power_off();

				for (k = 0; k < 8; k++)
					ADS_Temp_Data[k] = 0;

				FAN_Warning[0] = 0;
				FAN_Warning[1] = 0;
				DMD_Temp = 0;
				C381ModeChange = TRUE;
				video_src_changed = TRUE;
				it66352Init = FALSE;
				it6635_dev_restart();
			}
			else if (i2cRxBuffer[1] == 0xF3) // power on
			{
				// NOTE: mstar下电后，仿真机可能导致i2c2挂死，重新整机上电后需要重置i2c2
				MX_I2C2_Init();   
				MX_USART1_UART_Init();
				// HAL_UART_Receive_IT(&huart1, (uint8_t *)g_uart1_rx_buf, 18);

				Work_Stat = 1;				
				Warning_Update = 1 ;	// 触发状态上报
				mstar_work_on_oneshot_flag = 1; 
				power_monitor_on_power_on();
			}
			PJLink_StatusNotify_PowerStatus(Work_Stat);
			sprintf(Work_Status, "%d", Work_Stat);
		}
		break;
		case I2C_VIDEO_IN_CHAN: // VIDEO IN CHANNEL
		{
			Source_Stat = (VIDEO_INPUT_SRC_)(i2cRxBuffer[1]);
			printf("Video input channel: 0x%x\r\n", Source_Stat);
			sprintf(Source_Status, "%d", Source_Stat);
			video_src_changed = TRUE;

			if(Source_Stat == VIDEO_INPUT_SRC_HDMI_1 || Source_Stat == VIDEO_INPUT_SRC_HDMI_2)
			{
				video_src_changed = TRUE;
				rt_param.video_in_chan = Source_Stat;
				rt_param.input_source = Source_Stat;
			}
			save_rt_param_to_flash();
			PJLink_StatusNotify_InputStatus(Source_Stat);
		}
		break;
		// case I2C_Err_Read: // read mcu fan/temp error
		// {
		// 	I2C_CMD = i2cRxBuffer[1];
		// 	if (I2C_CMD == 0x40)
		// 		I2C_Read_in = TRUE;
		// 	ETH_IT_DISABLE();
		// 	HAL_I2C_SlaveTxCpltCallback(&hi2c1);
		// 	ETH_IT_ENABLE();
		// 	MX_I2C1_Init();
		// 	timeout = 0;
		// 	while (HAL_I2C_GetState(&hi2c1) != HAL_I2C_STATE_READY)
		// 	{
		// 		timeout++;
		// 		if (timeout > 0xFFFE)
		// 		{
		// 			printf("!HAL_I2C_STATE_READY\n");
		// 			timeout = 0;
		// 			break;
		// 		}
		// 		HAL_Delay(100);
		// 	}
		// 	timeout = 0;
		// 	//					 printf("I2Cs_CMD=0x%x\r\n",I2C_CMD);
		// 	if (I2C_CMD == 0x48)
		// 	{
		// 		log_err("FAN/Temp Refresh\r\n");
		// 		I2C_Read_in = FALSE;
		// 		Error_IRQ_Reset();
		// 	}
		// 	I2C_CMD = 0;
		// }
		// break;
		case I2C_OE_Lock: // read mcu fan/temp error
		{
			if (i2cRxBuffer[1] == 0x22)
			{
				OE_lock = 0;
				rt_param.lens_lock = 0;
			}
			else if (i2cRxBuffer[1] == 0x11)
			{
				OE_lock = TRUE;
				rt_param.lens_lock = 1;
			}	
			printf("lens_lock=%d\r\n", OE_lock);
			save_rt_param_to_flash();
			if (OE_lock == 1) //&&OE_Shift_Type==OE_Shift_RST)
			{
				OE_ShiftLR_stop();
				OE_ShiftUD_stop();
				OE_UD_Reset = 0;
				OE_LR_Reset = 0;
				OE_Focus_stop();
				OE_Zoom_stop();
				OE_Value_clean();
			}
		}
		break;
		case I2C_OE_Zoom: // read mcu fan/temp error
		{
			if (OE_lock == 1 || OE_Shift_Type == OE_Shift_RST)
				break;
			if (OE_Shift_Type != OE_Zoom)
			{
				OE_ShiftLR_stop();
				OE_Focus_stop();
				OE_ShiftUD_stop();
				OE_Value_clean();
			}
			OE_movement++;
			OE_Shift_Type = OE_Zoom;
			OE_Shift_dir = i2cRxBuffer[1];
			OE_Zoom_Enable();
			printf("I2C_OE_Zoom=%d\r\n", OE_movement);
		}
		break;
		case I2C_OE_Focus: // read mcu fan/temp error
		{
			if (OE_lock == 1 || OE_Shift_Type == OE_Shift_RST)
				break;
			if (OE_Shift_Type != OE_Focus)
			{
				OE_ShiftLR_stop();
				OE_Zoom_stop();
				OE_ShiftUD_stop();
				OE_Value_clean();
			}

			OE_movement++;
			OE_Shift_Type = OE_Focus;
			OE_Shift_dir = i2cRxBuffer[1];
			OE_Focus_Enable();
			printf("I2C_OE_Focus=%d\r\n", OE_movement);
		}
		break;
		case I2C_OE_UD: // read mcu fan/temp error
		{
			if (OE_lock == 1 || OE_Shift_Type == OE_Shift_RST)
				break;
			if (OE_Shift_Type != OE_Shift_UD)
			{
				OE_ShiftLR_stop();
				OE_Zoom_stop();
				OE_Focus_stop();
				OE_Value_clean();
			}
			OE_movement++;
			OE_Shift_Type = OE_Shift_UD;
			OE_Shift_dir = i2cRxBuffer[1];
			OE_ShiftUD_Enable();
			printf("I2C_OE_UD=%d\r\n", OE_movement);
		}
		break;
		case I2C_OE_LR: // read mcu fan/temp error
		{
			if (OE_lock == 1 || OE_Shift_Type == OE_Shift_RST)
				break;
			if (OE_Shift_Type != OE_Shift_LR)
			{
				OE_ShiftUD_stop();
				OE_Zoom_stop();
				OE_Focus_stop();
				OE_Value_clean();
			}
			OE_movement++;
			OE_Shift_Type = OE_Shift_LR;
			OE_Shift_dir = i2cRxBuffer[1];
			OE_ShiftLR_Enable();
			printf("I2C_OE_LR=%d\r\n", OE_movement);
		}
		break;
		case I2C_OE_Rst: // read mcu fan/temp error
		{
			if (OE_lock == 1 || OE_Shift_Type == OE_Shift_RST)
				break;
			OE_Shift_Type = OE_Shift_RST;
			OE_LR_Reset = 0;
			OE_UD_Reset = 0;

			HAL_GPIO_WritePin(SLEEP0_MOTOR_GPIO_Port, SLEEP0_MOTOR_Pin, GPIO_PIN_SET); // test

			// HAL_GPIO_WritePin(SLEEP_MOTOR2_GPIO_Port,  SLEEP_MOTOR2_Pin, GPIO_PIN_SET);
			motor_nsleep = TRUE;
			printf("I2C_OE_Rst\n");
		}
		break;
		case I2C_Warn_Rep: // 0x91 
		{
			printf("I2C_Warn_Rep=0x%x\r\n", i2cRxBuffer[1]);
			if (i2cRxBuffer[1] == 0x11)
			{
				Warning_Update = 1;
				printf("Warning_Update\n");
			}
			break;
		}
		default:
			break;
		} // switch() end
		i2cRxBuffer[0] = 0;
		i2cRxBuffer[1] = 0;
	} // if HAL_I2C_Slave_Receive_IT() end
	timeout = 0;
	while (HAL_OK != HAL_I2C_Slave_Receive_IT(&hi2c1, (uint8_t *)i2cRxBuffer, 2))
	{
		timeout++;
		if (timeout > 0xFFFE)
		{
			printf("I2C_R_R\n");
			I2C_Busy2 = 0x22;
			Reset_I2C();
			timeout = 0;
			break;
		}
		HAL_Delay(100);
	}
	timeout = 0;
	/******************************************************/
}

void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
{
	if (hi2c->Instance == I2C2)
		printf("hi2.Instance=I2C2 Error \n");
	else
		printf("hi2.Instance=I2C1 Error \n");

	printf("hi2c->ErrorCode=%d\n", hi2c->ErrorCode);
	if (hi2c->ErrorCode == HAL_I2C_ERROR_BERR)
	{
		//		I2C_OLD_Sat=HAL_TIMEOUT;
		printf("I2C_BERR\n");
		I2C_Busy3 = 0x33;
		Reset_I2C();
	}
}


void lensCtrlStep(uint8_t lensOps, uint8_t param)
{
	switch(lensOps)
	{
		case I2C_OE_Lock: // read mcu fan/temp error
		{
			if (param == LENS_UNLOCK)
				OE_lock = 0;
			else if (param == LENS_LOCK)
				//  OE_lock=false;
				OE_lock = TRUE;
			printf("OE_lock=%d\r\n", OE_lock);
			if (OE_lock == 1) //&&OE_Shift_Type==OE_Shift_RST)
			{
				OE_ShiftLR_stop();
				OE_ShiftUD_stop();
				OE_UD_Reset = 0;
				OE_LR_Reset = 0;
				OE_Focus_stop();
				OE_Zoom_stop();
				OE_Value_clean();
			}
		}
		break;
		case I2C_OE_Zoom: // read mcu fan/temp error
		{
			if (OE_lock == 1 || OE_Shift_Type == OE_Shift_RST)
				break;
			if (OE_Shift_Type != OE_Zoom)
			{
				OE_ShiftLR_stop();
				OE_Focus_stop();
				OE_ShiftUD_stop();
				OE_Value_clean();
			}
			OE_movement++;
			OE_Shift_Type = OE_Zoom;
			OE_Shift_dir = param;
			OE_Zoom_Enable();
			printf("I2C_OE_Zoom=%d\r\n", OE_movement);
		}
		break;
		case I2C_OE_Focus: // read mcu fan/temp error
		{
			if (OE_lock == 1 || OE_Shift_Type == OE_Shift_RST)
				break;
			if (OE_Shift_Type != OE_Focus)
			{
				OE_ShiftLR_stop();
				OE_Zoom_stop();
				OE_ShiftUD_stop();
				OE_Value_clean();
			}

			OE_movement++;
			OE_Shift_Type = OE_Focus;
			OE_Shift_dir = param;
			OE_Focus_Enable();
			printf("I2C_OE_Focus=%d\r\n", OE_movement);
		}
		break;
		case I2C_OE_UD: // read mcu fan/temp error
		{
			if (OE_lock == 1 || OE_Shift_Type == OE_Shift_RST)
				break;
			if (OE_Shift_Type != OE_Shift_UD)
			{
				OE_ShiftLR_stop();
				OE_Zoom_stop();
				OE_Focus_stop();
				OE_Value_clean();
			}
			OE_movement++;
			OE_Shift_Type = OE_Shift_UD;
			OE_Shift_dir = param;
			OE_ShiftUD_Enable();
			printf("I2C_OE_UD=%d\r\n", OE_movement);
		}
		break;
		case I2C_OE_LR: // read mcu fan/temp error
		{
			if (OE_lock == 1 || OE_Shift_Type == OE_Shift_RST)
				break;
			if (OE_Shift_Type != OE_Shift_LR)
			{
				OE_ShiftUD_stop();
				OE_Zoom_stop();
				OE_Focus_stop();
				OE_Value_clean();
			}
			OE_movement++;
			OE_Shift_Type = OE_Shift_LR;
			OE_Shift_dir = param;
			OE_ShiftLR_Enable();
			printf("I2C_OE_LR=%d\r\n", OE_movement);
		}
		break;
		case I2C_OE_Rst: // read mcu fan/temp error
		{
			if (OE_lock == 1 || OE_Shift_Type == OE_Shift_RST)
				break;
			OE_Shift_Type = OE_Shift_RST;
			OE_LR_Reset = 0;
			OE_UD_Reset = 0;

			HAL_GPIO_WritePin(SLEEP0_MOTOR_GPIO_Port, SLEEP0_MOTOR_Pin, GPIO_PIN_SET); // test

			// HAL_GPIO_WritePin(SLEEP_MOTOR2_GPIO_Port,  SLEEP_MOTOR2_Pin, GPIO_PIN_SET);
			motor_nsleep = TRUE;
			printf("I2C_OE_Rst\n");
		}
		break;
	}
}

/* USER CODE END 1 */

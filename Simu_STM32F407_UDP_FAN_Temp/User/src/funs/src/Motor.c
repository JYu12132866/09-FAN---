#include "main.h"
#include "adc.h"
#include "Motor.h"
#include "tim.h"
#include "usart.h"
#include "rt_param.h"

uint8_t  OE_Shift_dir;
uint8_t  OE_Shift_Count;
_Bool  OE_LR_Reset;
_Bool  OE_UD_Reset;
uint8_t  OE_Focus_dir;
uint8_t  OE_Focus_Count;
_Bool  OE_Focus_Reset;
uint8_t  OE_Zoom_dir;
uint8_t  OE_Zoom_Count;
_Bool  OE_Zoom_Reset;
uint8_t motor_Shift=0;
_Bool motor_nsleep;
uint8_t motor_FZ;
uint8_t shift_dir=0;
uint16_t shift_rst_cnt;
uint8_t shift_LR_rst;
uint8_t shift_UP_rst;
extern uint8_t OE_timer_cnt;

#if 1


void OE_Value_clean(void)
{
	 if(motor_nsleep!=FALSE)
		  motor_nsleep=FALSE;
	 if(motor_Shift!=0)
		    motor_Shift=0;
	 if(OE_Shift_Type!=0)
		   OE_Shift_Type=0;
	 if(OE_Shift_dir!=0)
		  OE_Shift_dir=0;
	 if(OE_timer_cnt!=IR_Rep2)
		   OE_timer_cnt=IR_Rep2;
	 if(motor_FZ!=0)
  		motor_FZ=0;
	 if(OE_movement!=0)
		  OE_movement=0;
	
}
void OE_Shift_Handler(void)
{
	if (OE_Shift_Type == OE_Shift_UD)
	{
		// if(motor_FZ>=3)
		{
			// printf(" VERTICAL_EN: %.3fV\r\n", OE_UD_Average);
			OE_timer_cnt = IR_Rep3;
			if (OE_lmt != 0x33 && OE_lmt != 0x44)
			{
				HAL_TIM_PWM_Start(UD_MOTOR_TIM, UD_MOTOR_TIM_CHANNEL);
				HAL_GPIO_WritePin(MOTOR2_ENABLE_GPIO_Port, MOTOR2_ENABLE_Pin, GPIO_PIN_RESET);
			}
		}
	}
	else if (OE_Shift_Type == OE_Shift_LR)
	{
		// if(motor_FZ>=3)
		{
			// printf(" HORIZONTAL_EN: %.3fV\r\n", OE_LR_Average);
			OE_timer_cnt = IR_Rep3;
			if (OE_lmt != 0x11 && OE_lmt != 0x22)
			{
				printf("start motor LR\r\n");
				HAL_TIM_PWM_Start(LR_MOTOR_TIM, LR_MOTOR_TIM_CHANNEL);
				HAL_GPIO_WritePin(MOTOR3_ENABLE_GPIO_Port, MOTOR3_ENABLE_Pin, GPIO_PIN_RESET);
			}
		}
	}
	else if (OE_Shift_Type == OE_Zoom)
	{
		if (OE_Shift_dir == 0x11)
		{
			HAL_GPIO_WritePin(MOTOR0_AIN2_GPIO_Port, MOTOR0_AIN2_Pin, GPIO_PIN_SET);
			if (motor_FZ >= 3)
			{
				OE_timer_cnt = IR_Rep3;
				HAL_GPIO_WritePin(MOTOR0_AIN1_GPIO_Port, MOTOR0_AIN1_Pin, GPIO_PIN_RESET);
			}
			else
				HAL_GPIO_TogglePin(MOTOR0_AIN1_GPIO_Port, MOTOR0_AIN1_Pin);
		}
		else if (OE_Shift_dir == 0x22)
		{
			HAL_GPIO_WritePin(MOTOR0_AIN1_GPIO_Port, MOTOR0_AIN1_Pin, GPIO_PIN_SET);
			if (motor_FZ >= 3)
			{
				OE_timer_cnt = IR_Rep3;
				HAL_GPIO_WritePin(MOTOR0_AIN2_GPIO_Port, MOTOR0_AIN2_Pin, GPIO_PIN_RESET);
			}
			else
				HAL_GPIO_TogglePin(MOTOR0_AIN2_GPIO_Port, MOTOR0_AIN2_Pin);
		}
		else if (OE_Shift_dir == 0x33)
		{
			HAL_GPIO_WritePin(MOTOR0_AIN1_GPIO_Port, MOTOR0_AIN1_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(MOTOR0_AIN2_GPIO_Port, MOTOR0_AIN2_Pin, GPIO_PIN_RESET);
		}
	}
	else if (OE_Shift_Type == OE_Focus)
	{
		if (OE_Shift_dir == 0x45)
		{
			HAL_GPIO_WritePin(MOTOR0_BIN2_GPIO_Port, MOTOR0_BIN2_Pin, GPIO_PIN_SET);
			if (motor_FZ >= 3)
			{
				OE_timer_cnt = IR_Rep3;
				HAL_GPIO_WritePin(MOTOR0_BIN1_GPIO_Port, MOTOR0_BIN1_Pin, GPIO_PIN_RESET);
			}
			else
				HAL_GPIO_TogglePin(MOTOR0_BIN1_GPIO_Port, MOTOR0_BIN1_Pin);
		}
		else if (OE_Shift_dir == 0x44)
		{
			HAL_GPIO_WritePin(MOTOR0_BIN1_GPIO_Port, MOTOR0_BIN1_Pin, GPIO_PIN_SET);
			if (motor_FZ >= 3)
			{
				OE_timer_cnt = IR_Rep3;
				HAL_GPIO_WritePin(MOTOR0_BIN2_GPIO_Port, MOTOR0_BIN2_Pin, GPIO_PIN_RESET);
			}
			else
				HAL_GPIO_TogglePin(MOTOR0_BIN2_GPIO_Port, MOTOR0_BIN2_Pin);
		}
		else if (OE_Shift_dir == 0x33)
		{
			HAL_GPIO_WritePin(MOTOR0_BIN1_GPIO_Port, MOTOR0_BIN1_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(MOTOR0_BIN2_GPIO_Port, MOTOR0_BIN2_Pin, GPIO_PIN_RESET);
		}
	}
}
#endif

void OE_Shift_Reset(void)
{
	// 默认旧马达模式
	int dir_up_right_pin_lvl  =  GPIO_PIN_SET;
	int dir_down_left_pin_lvl =  GPIO_PIN_RESET;
	// 新马达方向pin lvl 定义
	if(rt_param.shift_motor_model == SHIFT_MOTOR_DZ_24BYJ48_A14)
	{
		dir_up_right_pin_lvl  =  GPIO_PIN_RESET;
		dir_down_left_pin_lvl =  GPIO_PIN_SET;
	}

	if (OE_ave_set)
	{
		if (!OE_LR_Reset && !OE_UD_Reset)
		{
			if (OE_UD_Average > (ShiftV_Center + 0.02f)) // move down
			{
				if (shift_UP_rst != 0x11)
				{
					printf("shift_DW_rst\r\n");
					shift_rst_cnt = 0;
					shift_UP_rst = 0x11;
				}
				HAL_TIM_PWM_Start(UD_MOTOR_TIM, UD_MOTOR_TIM_CHANNEL);
				HAL_GPIO_WritePin(MOTOR2_ENABLE_GPIO_Port, MOTOR2_ENABLE_Pin, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(MOTOR2_DIR_GPIO_Port, MOTOR2_DIR_Pin, dir_down_left_pin_lvl);
				HAL_GPIO_WritePin(MOTOR2_RESET_GPIO_Port, MOTOR2_RESET_Pin, GPIO_PIN_SET);
				shift_rst_cnt++;
				printf("shift_DW_cnt=%d\r\n", shift_rst_cnt);
			}
			else if (OE_UD_Average < (ShiftV_Center - 0.02f)) // move up
			{
				if (shift_UP_rst != 0x22)
				{
					printf("shift_UP_rst\r\n");
					shift_rst_cnt = 0;
					shift_UP_rst = 0x22;
				}
				HAL_TIM_PWM_Start(UD_MOTOR_TIM, UD_MOTOR_TIM_CHANNEL);
				HAL_GPIO_WritePin(MOTOR2_ENABLE_GPIO_Port, MOTOR2_ENABLE_Pin, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(MOTOR2_DIR_GPIO_Port, MOTOR2_DIR_Pin, dir_up_right_pin_lvl);
				HAL_GPIO_WritePin(MOTOR2_RESET_GPIO_Port, MOTOR2_RESET_Pin, GPIO_PIN_SET);
				shift_rst_cnt++;
				printf("shift_UP_cnt=%d\r\n", shift_rst_cnt);
			}
			else
			{
				HAL_TIM_PWM_Stop(UD_MOTOR_TIM, UD_MOTOR_TIM_CHANNEL);
				HAL_GPIO_WritePin(MOTOR2_ENABLE_GPIO_Port, MOTOR2_ENABLE_Pin, GPIO_PIN_SET);
				HAL_GPIO_WritePin(MOTOR2_RESET_GPIO_Port, MOTOR2_RESET_Pin, GPIO_PIN_RESET);
				printf("shift_UP_rst_done\r\n");
				shift_rst_cnt = 0;
				shift_UP_rst = 0;
				OE_LR_Reset = 1;
			}
		}
		else if (!OE_UD_Reset && OE_LR_Reset)
		{
			if (OE_LR_Average > (ShiftH_Center + 0.02f)) // move right
			{
				if (shift_LR_rst != 0x11)
				{
					printf("shift_right_rst\r\n");
					shift_rst_cnt = 0;
					shift_LR_rst = 0x11;
				}
				HAL_TIM_PWM_Start(LR_MOTOR_TIM, LR_MOTOR_TIM_CHANNEL);
				HAL_GPIO_WritePin(MOTOR3_ENABLE_GPIO_Port, MOTOR3_ENABLE_Pin, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(MOTOR3_DIR_GPIO_Port, MOTOR3_DIR_Pin, dir_up_right_pin_lvl);
				HAL_GPIO_WritePin(MOTOR2_RESET_GPIO_Port, MOTOR2_RESET_Pin, GPIO_PIN_SET);
				shift_rst_cnt++;
				printf("shift_Right_cnt=%d\r\n", shift_rst_cnt);
			}
			else if (OE_LR_Average < (ShiftH_Center - 0.02f)) // move left
			{
				if (shift_LR_rst != 0x22)
				{
					printf("shift_left_rst\r\n");
					shift_rst_cnt = 0;
					shift_LR_rst = 0x22;
				}
				HAL_TIM_PWM_Start(LR_MOTOR_TIM, LR_MOTOR_TIM_CHANNEL);
				HAL_GPIO_WritePin(MOTOR3_ENABLE_GPIO_Port, MOTOR3_ENABLE_Pin, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(MOTOR3_DIR_GPIO_Port, MOTOR3_DIR_Pin, dir_down_left_pin_lvl);
				HAL_GPIO_WritePin(MOTOR2_RESET_GPIO_Port, MOTOR2_RESET_Pin, GPIO_PIN_SET);
				shift_rst_cnt++;
				printf("shift_left_cnt=%d\r\n", shift_rst_cnt);
			}
			else
			{
				HAL_TIM_PWM_Stop(LR_MOTOR_TIM, LR_MOTOR_TIM_CHANNEL);
				HAL_GPIO_WritePin(MOTOR3_ENABLE_GPIO_Port, MOTOR3_ENABLE_Pin, GPIO_PIN_SET);
				HAL_GPIO_WritePin(MOTOR2_RESET_GPIO_Port, MOTOR2_RESET_Pin, GPIO_PIN_RESET);
				printf("shift_LR_rst_done\r\n");
				shift_rst_cnt = 0;
				shift_LR_rst = 0;
				OE_UD_Reset = 1;
			}
		}
	}

	if ((OE_UD_Reset && OE_LR_Reset) || shift_rst_cnt > 680)
	{
		HAL_GPIO_WritePin(MOTOR3_ENABLE_GPIO_Port, MOTOR3_ENABLE_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(MOTOR2_ENABLE_GPIO_Port, MOTOR2_ENABLE_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(MOTOR2_RESET_GPIO_Port, MOTOR2_RESET_Pin, GPIO_PIN_RESET);
		HAL_TIM_PWM_Stop(UD_MOTOR_TIM, UD_MOTOR_TIM_CHANNEL);
		HAL_TIM_PWM_Stop(LR_MOTOR_TIM, LR_MOTOR_TIM_CHANNEL);
		shift_rst_cnt = 0;
		OE_UD_Reset = 0;
		OE_LR_Reset = 0;
		shift_LR_rst = 0;
		OE_Value_clean();
		HAL_UART_Transmit(&huart1, (uint8_t *)"UFm", 3, 500); 	// Announce Mstar OE reset done
		printf("OE_Shift_Reset_done\r\n");
	}
}

void OE_ShiftUD_stop(void)
{
	HAL_GPIO_WritePin(MOTOR2_ENABLE_GPIO_Port, MOTOR2_ENABLE_Pin, GPIO_PIN_SET);
	HAL_TIM_PWM_Stop(UD_MOTOR_TIM, UD_MOTOR_TIM_CHANNEL);
	HAL_GPIO_WritePin(MOTOR2_RESET_GPIO_Port, MOTOR2_RESET_Pin, GPIO_PIN_RESET);
}
void OE_ShiftLR_stop(void)
{
	HAL_GPIO_WritePin(MOTOR3_ENABLE_GPIO_Port, MOTOR3_ENABLE_Pin, GPIO_PIN_SET);
	HAL_TIM_PWM_Stop(LR_MOTOR_TIM, LR_MOTOR_TIM_CHANNEL);
	HAL_GPIO_WritePin(MOTOR2_RESET_GPIO_Port, MOTOR2_RESET_Pin, GPIO_PIN_RESET);
}

void OE_Focus_stop(void)
{
	HAL_GPIO_WritePin(MOTOR0_BIN1_GPIO_Port, MOTOR0_BIN1_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(MOTOR0_BIN2_GPIO_Port, MOTOR0_BIN2_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(SLEEP0_MOTOR_GPIO_Port, SLEEP0_MOTOR_Pin, GPIO_PIN_RESET);
}
void OE_Zoom_stop(void)
{
	HAL_GPIO_WritePin(MOTOR0_AIN1_GPIO_Port, MOTOR0_AIN1_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(MOTOR0_AIN2_GPIO_Port, MOTOR0_AIN2_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(SLEEP0_MOTOR_GPIO_Port, SLEEP0_MOTOR_Pin, GPIO_PIN_RESET);
}

void OE_Zoom_Enable(void)
{
	HAL_GPIO_WritePin(SLEEP0_MOTOR_GPIO_Port, SLEEP0_MOTOR_Pin, GPIO_PIN_SET);
	motor_Shift = 0x23;
	motor_nsleep = TRUE;
}
void OE_Focus_Enable(void)
{
	HAL_GPIO_WritePin(SLEEP0_MOTOR_GPIO_Port, SLEEP0_MOTOR_Pin, GPIO_PIN_SET);
	motor_Shift = 0x32;
	motor_nsleep = TRUE;
}

void OE_ShiftUD_Enable(void)
{
	// 默认旧马达模式
	int dir_up_right_pin_lvl  =  GPIO_PIN_SET;
	int dir_down_left_pin_lvl =  GPIO_PIN_RESET;
	// 新马达方向pin lvl 定义
	if(rt_param.shift_motor_model == SHIFT_MOTOR_DZ_24BYJ48_A14)
	{
		dir_up_right_pin_lvl  =  GPIO_PIN_RESET;
		dir_down_left_pin_lvl =  GPIO_PIN_SET;
	}

	if (OE_Shift_dir == 0x11) // down
	{
		HAL_GPIO_WritePin(MOTOR2_DIR_GPIO_Port, MOTOR2_DIR_Pin, dir_down_left_pin_lvl);
	}
	else if (OE_Shift_dir == 0x22) // up
	{
		HAL_GPIO_WritePin(MOTOR2_DIR_GPIO_Port, MOTOR2_DIR_Pin, dir_up_right_pin_lvl);
	}
	else if (OE_Shift_dir == 0x33)
	{
		HAL_GPIO_WritePin(MOTOR2_ENABLE_GPIO_Port, MOTOR2_ENABLE_Pin, GPIO_PIN_SET);
	}

	HAL_GPIO_WritePin(MOTOR2_RESET_GPIO_Port, MOTOR2_RESET_Pin, GPIO_PIN_SET);
	motor_nsleep = TRUE;
	motor_Shift = 0x4A;
}
void OE_ShiftLR_Enable(void)
{
	// 默认旧马达模式
	int dir_up_right_pin_lvl  =  GPIO_PIN_SET;
	int dir_down_left_pin_lvl =  GPIO_PIN_RESET;
	// 新马达方向pin lvl 定义
	if(rt_param.shift_motor_model == SHIFT_MOTOR_DZ_24BYJ48_A14)
	{
		dir_up_right_pin_lvl  =  GPIO_PIN_RESET;
		dir_down_left_pin_lvl =  GPIO_PIN_SET;
	}

	if (OE_Shift_dir == 0x11) // right
	{
		HAL_GPIO_WritePin(MOTOR3_DIR_GPIO_Port, MOTOR3_DIR_Pin, dir_up_right_pin_lvl);
	}
	else if (OE_Shift_dir == 0x22) // left
	{
		HAL_GPIO_WritePin(MOTOR3_DIR_GPIO_Port, MOTOR3_DIR_Pin, dir_down_left_pin_lvl);
	}
	else if (OE_Shift_dir == 0x33)
	{
		HAL_GPIO_WritePin(MOTOR3_ENABLE_GPIO_Port, MOTOR3_ENABLE_Pin, GPIO_PIN_SET);
		// HAL_GPIO_WritePin(MOTOR3_ENABLE_GPIO_Port, MOTOR3_ENABLE_Pin, GPIO_PIN_RESET);
	}

	HAL_GPIO_WritePin(MOTOR2_RESET_GPIO_Port, MOTOR2_RESET_Pin, GPIO_PIN_SET);
	motor_nsleep = TRUE;
	motor_Shift = 0xA4;
	printf("OE LR enable\r\n");
}

/*  ************************************************    */

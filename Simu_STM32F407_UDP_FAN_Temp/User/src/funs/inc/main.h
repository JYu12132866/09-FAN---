/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
	
#include "pj_hw_def.h"

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
#define TRUE 1
#define FALSE 0

#define true  1
#define false 0

#define PASS 1
#define FAILED 0
	
#define ON 1
#define OFF 0

#define PWR_ON    0XF3
#define PWR_OFF   0X3F

// #define Ethernet 1
// #define PID_En 0
#define C381 1
/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */
#define I2C_XML_Enable    0x30
#define I2C_Report_Sel    0x31
#define I2C_Err_Read      0x44
#define I2C_DMD_Temp      0x59
#define I2C_Work_Sat      0x51
#define I2C_VIDEO_IN_CHAN 0x53
#define I2C_IP_ADJ        0x57

#define I2C_OE_Lock       0x62
#define I2C_OE_Zoom       0x63
#define I2C_OE_Focus      0x64
#define I2C_OE_LR         0x65
#define I2C_OE_UD         0x66
#define I2C_OE_Rst        0x67

#define LENS_FOCUS_FUTHER   0X45
#define LENS_FOCUS_CLOSER   0X44
#define LENS_ZOOM_PLUS      0X22
#define LENS_ZOOM_MINUS     0X11
#define LENS_SHIFT_LEFT     0X22
#define LENS_SHIFT_RIGHT    0X11
#define LENS_SHIFT_UP       0x22
#define LENS_SHIFT_DOWN     0x11
#define LENS_LOCK           0X11
#define LENS_UNLOCK         0X22

#define I2C_Worktime1     0x71
#define I2C_Worktime2     0x72
#define I2C_Worktime3     0x73
#define I2C_Worktime4     0x74

// #define I2C_Worktime2     0x75

#define I2C_WorkMode      0x78

//warping chip C381 control cmd
#define I2C_CurMode         0x80    //cursor mode, ?????
#define I2C_CurSel_LR       0x81    //cursor select left/right????????????   
#define I2C_CurSel_UD       0x82    //cursor select up/down????????????
#define I2C_Warp_LR         0x83    //warping left/right?????????
#define I2C_Warp_UD         0x84    //warping up/down?????????
#define I2C_Warp_Save       0x85    //warping save?????????????
#define I2C_Warp_Load       0x86    //warping load?????????????
#define I2C_Warp_Rst        0x87    //warping reset???????��
#define I2C_C381_Mode       0x88    //c381 Mode, ??????��?
#define I2C_Fake_Auto_Warp  0x89    //fake auto warp, for demo

// #define I2C_CurMode         0x70    //cursor mode, ?????
// #define I2C_CurSel_LR       0x81    //cursor select left/right????????????   
// #define I2C_CurSel_UD       0x82    //cursor select up/down????????????
// #define I2C_Warp_LR         0x83    //warping left/right?????????
// #define I2C_Warp_UD         0x84    //warping up/down?????????
// #define I2C_Warp_Save       0x72    //warping save?????????????
// #define I2C_Warp_Load       0x49    //warping load?????????????
// #define I2C_Warp_Rst        0x89    //warping reset???????��
// #define I2C_C381_Mode     0x88    //c381 bypass
//it66352 hdmi switch chip control cmd
#define I2C_HDMI_SWITCH     0x90  
#define I2C_Warn_Rep        0x91  

typedef enum{
  VIDEO_INPUT_SRC_AUTO = 0,
  VIDEO_INPUT_SRC_VGA,
  VIDEO_INPUT_SRC_DP_1,
  VIDEO_INPUT_SRC_HDMI_1,
  VIDEO_INPUT_SRC_DVI,
  VIDEO_INPUT_SRC_HDMI_2,
  VIDEO_INPUT_SRC_DP_2,
  VIDEO_INPUT_SRC_HD_BASE,
}VIDEO_INPUT_SRC_;

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

// IAP更新参数结构体定义
#define FLASH_IAP_PARAM_ADDR    ADDR_FLASH_SECTOR_3
typedef struct  
{
    uint8_t update_sts;         //更新状态机保存，bootloader使用
    uint8_t version_major;      //bootloader 版本
    uint8_t version_minor;
    uint8_t iapUpdateFlag;      //iap更新请求flag
    uint8_t remoteIp[4];        //请求更新的远端IP
    uint16_t remotePort;        //请求更新的远端PORT
    uint8_t rsv2[2];
}IAP_UPDATE_PARAM;
extern IAP_UPDATE_PARAM iapParam;

#define IP_ADDR0   192
#define IP_ADDR1   168
#define IP_ADDR2   0
#define IP_ADDR3   20

/*NETMASK*/
#define NETMASK_ADDR0   255
#define NETMASK_ADDR1   255
#define NETMASK_ADDR2   255
#define NETMASK_ADDR3   0

/*Gateway Address*/
#define GW_ADDR0   192
#define GW_ADDR1   168
#define GW_ADDR2   0
#define GW_ADDR3   1

/*Gateway Address*/
#define RemoteIP0   192
#define RemoteIP1   168
#define RemoteIP2   0
#define RemoteIP3   100

#define UDP_DEMO_PORT			8080	//????udp??????? 
/* MII and RMII mode selection, for STM324xG-EVAL Board(MB786) RevB ***********/
#define RMII_MODE // User have to provide the 50 MHz clock by soldering a 50 MHz
                  // oscillator (ref SM7745HEV-50.0M or equivalent) on the U3
                  // footprint located under CN3 and also removing jumper on JP5. 
                  // This oscillator is not provided with the board. 
                  // For more details, please refer to STM3240G-EVAL evaluation
                  // board User manual (UM1461).
/* USER CODE END EM */
#define APP_CODE_OFFSET 0x10000
#define RESET_IRQ_VEC_TAB_BASE()  \
  do { \
      SCB->VTOR = FLASH_BASE | APP_CODE_OFFSET; \
      /*__enable_irq(); NOTE: IAP?????????????APP code???bootloader????????��?*/\
  } while (0)

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);
// void Time_Update(void);
void ResetIP(void);
void lwipIPUpdate(void);
void ip_setting_update(uint8_t *ip, uint8_t *mask, uint8_t *gw);
int set_ip_config(char* ip, char* netmask, char* gw);
int get_ip_config(char* ip, char* netmask, char* gw);
int set_report_ip(char* ip, int port);
int get_report_ip(char* ip, int* port);

/* USER CODE BEGIN EFP */
extern _Bool OE_lock;
extern uint16_t OE_movement;
extern uint8_t I2C_CMD;
extern int Work_Stat;
extern char Work_Status[3];
extern char Source_Status[3];
extern signed char War_State;
// extern int Source_Stat;
extern VIDEO_INPUT_SRC_ Source_Stat;
extern _Bool video_src_changed;
extern _Bool it66352Init;

// extern uint8_t g_uart1_rx_cmd_buf[18];
extern uint8_t Ethernet_Info[20];

extern uint32_t secondCounter;
extern uint8_t waitMstAlive;
extern uint32_t waitMstStartTime;

extern IWDG_HandleTypeDef iwdg;
// extern __IO uint32_t LocalTime;
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
/* USER CODE BEGIN Private defines */

#if(Projector_Model == LP92SLB || Projector_Model == NP72BN)
  #define MCU_LED_GPIO_Port GPIOE
  #define MCU_LED_Pin GPIO_PIN_2
  #define FAN_LOCKD2_GPIO_Port GPIOC
  #define FAN_LOCKD2_Pin GPIO_PIN_15
  //Vertical Shift
  #define MOTOR2_STEP_GPIO_Port GPIOE 
  #define MOTOR2_STEP_Pin GPIO_PIN_5
  //Horizontal Shift 
  #define MOTOR3_DIR_GPIO_Port GPIOB 
  #define MOTOR3_DIR_Pin GPIO_PIN_8 
  #define MOTOR3_STEP_GPIO_Port GPIOE 
  #define MOTOR3_STEP_Pin GPIO_PIN_6
  //HDMI MUX Chip (ITE it66352) rst pin & int pin
  #define ITE_RST_GPIO_Port GPIOD
  #define ITE_RST_Pin GPIO_PIN_14
  #define ITE_INT_GPIO_Port GPIOD
  #define ITE_INT_Pin GPIO_PIN_15S

  #define FAN3_PWM_Pin GPIO_PIN_15
  #define FAN3_PWM_GPIO_Port GPIOA

  #define TEMP_R1_GPIO_Port GPIOA
  #define TEMP_R1_Pin GPIO_PIN_3
  #define TEMP_R2_GPIO_Port GPIOC
  #define TEMP_R2_Pin GPIO_PIN_0
  #define TEMP_G1_GPIO_Port GPIOC
  #define TEMP_G1_Pin GPIO_PIN_2
  #define TEMP_G2_GPIO_Port GPIOC
  #define TEMP_G2_Pin GPIO_PIN_3
  #define TEMP_B1_GPIO_Port GPIOA
  #define TEMP_B1_Pin GPIO_PIN_0
  #define TEMP_B2_GPIO_Port GPIOB
  #define TEMP_B2_Pin GPIO_PIN_0
  #define TEMP_AMB_GPIO_Port GPIOB
  #define TEMP_AMB_Pin GPIO_PIN_1
  
  #define C381_RST_GPIO_PORT   GPIOG
  #define C381_RST_GPIO_PIN    GPIO_PIN_2
  
  // #define C381_RST_GPIO_PORT   GPIOF
  // #define C381_RST_GPIO_PIN    GPIO_PIN_8
  // #define C381_INIT_GPIO_PORT  GPIOF
  // #define C381_INIT_GPIO_PIN   GPIO_PIN_4
  
#else
  #define MCU_LED_GPIO_Port GPIOE
  #define MCU_LED_Pin GPIO_PIN_2

  #define C381_RST_GPIO_PORT   GPIOF
  #define C381_RST_GPIO_PIN    GPIO_PIN_8
  #define C381_INIT_GPIO_PORT  GPIOF
  #define C381_INIT_GPIO_PIN   GPIO_PIN_4

  #define FAN_LOCKD1_GPIO_Port GPIOE
  #define FAN_LOCKD1_Pin GPIO_PIN_5
  #define FAN_LOCKD2_GPIO_Port GPIOE
  #define FAN_LOCKD2_Pin GPIO_PIN_6
  //Vertical Shift
  #define MOTOR2_STEP_GPIO_Port GPIOB
  #define MOTOR2_STEP_Pin GPIO_PIN_8

  //Horizontal Shift 
  #define MOTOR3_STEP_GPIO_Port GPIOB 
  #define MOTOR3_STEP_Pin GPIO_PIN_1 
  #define MOTOR3_DIR_GPIO_Port GPIOB 
  #define MOTOR3_DIR_Pin GPIO_PIN_0 
  //NOTE: pump reserved pin
  #define PUMP_EN0_GPIO_Port GPIOD
  #define PUMP_EN0_Pin GPIO_PIN_14
  #define PUMP_EN1_GPIO_Port GPIOD
  #define PUMP_EN1_Pin GPIO_PIN_15
  #define FAN3_PWM_Pin GPIO_PIN_0
  #define FAN3_PWM_GPIO_Port GPIOA

#endif

// #define TEST
#ifdef TEST  //????UART????,???????????��??
    #define MCU0_USART3_TX_Pin GPIO_PIN_10
    #define MCU0_USART3_TX_GPIO_Port  GPIOC
    #define MCU0_USART3_RX_Pin  GPIO_PIN_11
    #define MCU0_USART3_RX_GPIO_Port  GPIOC
#else
    #define MCU0_USART3_TX_Pin GPIO_PIN_10  //leo 
    #define MCU0_USART3_TX_GPIO_Port GPIOB //leo 
    #define MCU0_USART3_RX_Pin GPIO_PIN_9 //leo  
    #define MCU0_USART3_RX_GPIO_Port GPIOD //leo 
#endif

// Fan pin defination
#define FAN1_EN_GPIO_Port GPIOC //leo 
#define FAN1_EN_Pin GPIO_PIN_11 //leo 
#define FAN2_EN_GPIO_Port GPIOC
#define FAN2_EN_Pin GPIO_PIN_12
#define FAN3_EN_GPIO_Port GPIOC
#define FAN3_EN_Pin GPIO_PIN_13

#define SOC_IRQ_Pin GPIO_PIN_2
#define SOC_IRQ_GPIO_Port GPIOE


#define TMP_SDA_Pin GPIO_PIN_0
#define TMP_SDA_GPIO_Port GPIOF
#define TMP_SCL_Pin GPIO_PIN_1
#define TMP_SCL_GPIO_Port GPIOF
#define MCU_FAN_IRQ_Pin GPIO_PIN_2
#define MCU_FAN_IRQ_GPIO_Port GPIOF

#define TEC_CFB_Pin GPIO_PIN_3
#define TEC_CFB_GPIO_Port GPIOA
#define VERTICAL_EN_Pin GPIO_PIN_4
#define VERTICAL_EN_GPIO_Port GPIOA
#define HORIZONTAL_EN_Pin GPIO_PIN_5
#define HORIZONTAL_EN_GPIO_Port GPIOA

#define MCU0_IRQ_Pin GPIO_PIN_11
#define MCU0_IRQ_GPIO_Port GPIOF
#define ETH_PHY_IRQ_Pin GPIO_PIN_7
#define ETH_PHY_IRQ_GPIO_Port GPIOE
#define ETH_PHY_RST_Pin GPIO_PIN_8
#define ETH_PHY_RST_GPIO_Port GPIOE
#define FAN1_PWM_Pin GPIO_PIN_9
#define FAN1_PWM_GPIO_Port GPIOE
#define FAN2_PWM_Pin GPIO_PIN_11
#define FAN2_PWM_GPIO_Port GPIOE
#define FAN8_PWM_Pin GPIO_PIN_13
#define FAN8_PWM_GPIO_Port GPIOE
#define FAN9_PWM_Pin GPIO_PIN_14
#define FAN9_PWM_GPIO_Port GPIOE

#define MOTOR2_RESET_Pin GPIO_PIN_6  //Vertical Shift MOTOR2_RESET_Pin
#define MOTOR2_RESET_GPIO_Port GPIOA   //Vertical Shift MOTOR2_RESET_GPIO_Port

#define MOTOR2_ENABLE_Pin GPIO_PIN_11 //Vertical Shift  MOTOR2_ENABLE_Pin
#define MOTOR2_ENABLE_GPIO_Port GPIOB //Vertical Shift MOTOR2_ENABLE_GPIO_Port

#define MOTOR2_DIR_Pin GPIO_PIN_12 ////Vertical Shift  MOTOR2_DIR_Pin
#define MOTOR2_DIR_GPIO_Port GPIOA ////Vertical Shift MOTOR2_DIR_GPIO_Port

#define MOTOR3_ENABLE_Pin GPIO_PIN_10 // Horizental Shift  
#define MOTOR3_ENABLE_GPIO_Port GPIOC // //Horizental Shift 

#define TEC_EN_Pin GPIO_PIN_10
#define TEC_EN_GPIO_Port GPIOD
#define TEC_PWM_Pin GPIO_PIN_12
#define TEC_PWM_GPIO_Port GPIOD
#define FAN10_PWM_Pin GPIO_PIN_13
#define FAN10_PWM_GPIO_Port GPIOD

#define MOTOR0_AIN1_Pin GPIO_PIN_6
#define MOTOR0_AIN1_GPIO_Port GPIOC
#define MOTOR0_AIN2_Pin GPIO_PIN_7
#define MOTOR0_AIN2_GPIO_Port GPIOC
#define MOTOR0_BIN1_Pin GPIO_PIN_8
#define MOTOR0_BIN1_GPIO_Port GPIOC
#define MOTOR0_BIN2_Pin GPIO_PIN_9
#define MOTOR0_BIN2_GPIO_Port GPIOC
#define SLEEP0_MOTOR_Pin GPIO_PIN_8
#define SLEEP0_MOTOR_GPIO_Port GPIOA
#define MCU0_USART1_TX_Pin GPIO_PIN_9
#define MCU0_USART1_TX_GPIO_Port GPIOA
#define MCU0_USART1_RX_Pin GPIO_PIN_10
#define MCU0_USART1_RX_GPIO_Port GPIOA

#define FAN4_PWM_Pin GPIO_PIN_3
#define FAN4_PWM_GPIO_Port GPIOB
#define FAN6_PWM_Pin GPIO_PIN_4
#define FAN6_PWM_GPIO_Port GPIOB
#define FAN7_PWM_Pin GPIO_PIN_5
#define FAN7_PWM_GPIO_Port GPIOB
#define MCU0_I2C1_SCL_Pin GPIO_PIN_6
#define MCU0_I2C1_SCL_GPIO_Port GPIOB
#define MCU0_I2C1_SDA_Pin GPIO_PIN_7
#define MCU0_I2C1_SDA_GPIO_Port GPIOB


#define FAN5_PWM_Pin GPIO_PIN_9
#define FAN5_PWM_GPIO_Port GPIOB

// #if C381
  #define IPC381_CS_Pin GPIO_PIN_12
  #define IPC381_CS_GPIO_Port GPIOB
  #define MCU0_SPI2_SCK_Pin GPIO_PIN_13
  #define MCU0_SPI2_SCK_GPIO_Port GPIOB
  #define MCU0_SPI2_MISO_Pin GPIO_PIN_14
  #define MCU0_SPI2_MISO_GPIO_Port GPIOB
  #define MCU0_SPI2_MOSI_Pin GPIO_PIN_15
  #define MCU0_SPI2_MOSI_GPIO_Port GPIOB
// #endif
#define C381_CS_LOW()       HAL_GPIO_WritePin(IPC381_CS_GPIO_Port, IPC381_CS_Pin,GPIO_PIN_RESET)
#define C381_CS_HIGH()      HAL_GPIO_WritePin(IPC381_CS_GPIO_Port, IPC381_CS_Pin,GPIO_PIN_SET)
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

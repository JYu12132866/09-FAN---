/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    usart.h
  * @brief   This file contains all the function prototypes for
  *          the usart.c file
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
#ifndef __USART_H__
#define __USART_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart3;

// extern uint8_t rx_temp;
extern uint8_t g_uart1_idx;
// extern uint8_t g_uart1_detected;  // decode command flag
// extern uint16_t g_uart1_checkTick;

extern uint8_t g_uart1_ack_flag;
extern uint8_t g_uart1_cmd_flag;
extern uint8_t g_uart1_ack_len;
extern uint8_t g_uart1_cmd_len;
extern uint8_t g_uart1_cmd_cmd;
extern uint8_t g_uart1_cmd_type;

/** 等待ACK包上下文 */
typedef struct{
  uint8_t waitAckFlag;
  uint8_t cmd;
  uint8_t *ackBuf;
  uint8_t ackBufLen;
  uint8_t ackFlag;
  uint8_t ackLen;
}waitAckCtx_t;
extern waitAckCtx_t g_waitAckCtx;


extern uint8_t g_uart1_rx_buf[256];       
extern uint8_t g_uart1_rx_cmd_buf[256];       
extern uint8_t g_uart1_rx_ack_buf[256]; 

extern uint8_t g_uart3_idx;
extern uint8_t uart3_rx_buf[256];
// extern uint8_t g_uart3_detected;  // decode command flag
// extern uint16_t g_uart3_checkTick;


// extern uint16_t RcvCount;
extern uint8_t  RcvErr;

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

void MX_USART1_UART_Init(void);
void MX_USART3_UART_Init(void);

/* USER CODE BEGIN Prototypes */

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __USART_H__ */


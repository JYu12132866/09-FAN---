/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    usart.c
 * @brief   This file provides code for the configuration
 *          of the USART instances.
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
#include "main.h"
#include "lwip_comm.h"
#include "stmflash.h"
#include "lan8720.h"
#include "udp_demo.h"
#include "it6635_import.h"
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "usart.h"
#include "declare_gFunc.h"
#include "utils/inc/util.h"

/* USER CODE BEGIN 0 */
#include <stdio.h>
#define huart_debug_x huart3

uint8_t g_uart1_rx_buf[256];        /*  uart1 接收buffer */
uint8_t g_uart1_idx;                /*  uart1 接收buffer索引 */

uint8_t g_uart1_rx_cmd_buf[256];    /*　命令包暂存buffer　*/
uint8_t g_uart1_cmd_flag;           /* uart1 cmd包标记 */
uint8_t g_uart1_cmd_len;            /* uart1 cmd包长度*/
uint8_t g_uart1_cmd_cmd;            /* uart1 cmd包命令 */
uint8_t g_uart1_cmd_type;           /* 0XAA 和 0XCC 两类协议类型标记 */

uint8_t g_uart1_rx_ack_buf[256];    /*  响应包暂存buffer  */
uint8_t g_uart1_ack_flag;           /* uart1 ack包标记 */
uint8_t g_uart1_ack_len;            /* uart1 ack包长度*/

uint8_t uart3_rx_buf[256];
uint8_t g_uart3_idx;

waitAckCtx_t g_waitAckCtx;

uint8_t RcvErr;

void lwipIPUpdate(void);
/* USER CODE END 0 */

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart3;

/* USART1 init function */

void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  // huart1.Init.BaudRate = 9600;
  huart1.Init.BaudRate = 19200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */
}
/* USART3 init function */

void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */
}

void HAL_UART_MspInit(UART_HandleTypeDef *uartHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if (uartHandle->Instance == USART1)
  {
    /* USER CODE BEGIN USART1_MspInit 0 */

    /* USER CODE END USART1_MspInit 0 */
    /* USART1 clock enable */
    __HAL_RCC_USART1_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**USART1 GPIO Configuration
    PA9     ------> USART1_TX
    PA10     ------> USART1_RX
    */
    GPIO_InitStruct.Pin = MCU0_USART1_TX_Pin | MCU0_USART1_RX_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* USART1 interrupt Init */
    HAL_NVIC_SetPriority(USART1_IRQn, 1, 1);
    HAL_NVIC_EnableIRQ(USART1_IRQn);
    /* USER CODE BEGIN USART1_MspInit 1 */

    __HAL_UART_ENABLE_IT(uartHandle, UART_IT_RXNE); /* 使能UART1接收中断 */
    __HAL_UART_ENABLE_IT(uartHandle, UART_IT_IDLE); // 开启串口的空闲中断

    /* USER CODE END USART1_MspInit 1 */
  }
  else if (uartHandle->Instance == USART3)
  {
    /* USER CODE BEGIN USART3_MspInit 0 */
    // __HAL_RCC_GPIOC_CLK_ENABLE();
    /* USER CODE END USART3_MspInit 0 */
    /* USART3 clock enable */
    __HAL_RCC_USART3_CLK_ENABLE();

    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    /**USART3 GPIO Configuration
    PB10     ------> USART3_TX
    PD9     ------> USART3_RX
    */
    GPIO_InitStruct.Pin = MCU0_USART3_TX_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART3;
    HAL_GPIO_Init(MCU0_USART3_TX_GPIO_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = MCU0_USART3_RX_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART3;
    HAL_GPIO_Init(MCU0_USART3_RX_GPIO_Port, &GPIO_InitStruct);

    /* USART3 interrupt Init */
    HAL_NVIC_SetPriority(USART3_IRQn, 3, 1);
    HAL_NVIC_EnableIRQ(USART3_IRQn);
    /* USER CODE BEGIN USART3_MspInit 1 */

    /* USER CODE END USART3_MspInit 1 */
  }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef *uartHandle)
{

  if (uartHandle->Instance == USART1)
  {
    /* USER CODE BEGIN USART1_MspDeInit 0 */

    /* USER CODE END USART1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART1_CLK_DISABLE();

    /**USART1 GPIO Configuration
    PA9     ------> USART1_TX
    PA10     ------> USART1_RX
    */
    HAL_GPIO_DeInit(GPIOA, MCU0_USART1_TX_Pin | MCU0_USART1_RX_Pin);

    /* USART1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USART1_IRQn);
    /* USER CODE BEGIN USART1_MspDeInit 1 */

    /* USER CODE END USART1_MspDeInit 1 */
  }
  else if (uartHandle->Instance == USART3)
  {
    /* USER CODE BEGIN USART3_MspDeInit 0 */

    /* USER CODE END USART3_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART3_CLK_DISABLE();

    /**USART3 GPIO Configuration
    PB10     ------> USART3_TX
    PD9     ------> USART3_RX
    */
    HAL_GPIO_DeInit(MCU0_USART3_TX_GPIO_Port, MCU0_USART3_TX_Pin);

    HAL_GPIO_DeInit(MCU0_USART3_RX_GPIO_Port, MCU0_USART3_RX_Pin);

    /* USART3 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USART3_IRQn);
    /* USER CODE BEGIN USART3_MspDeInit 1 */

    /* USER CODE END USART3_MspDeInit 1 */
  }
}


/* uart1  mstar -> mcu 接收中断 */
void USART1_IRQHandler(void)
{
    if (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_RXNE) != RESET) // 串口接收中断
    {
        HAL_UART_Receive(&huart1, &(g_uart1_rx_buf[g_uart1_idx++]), 1, 100); // 串口2接收1位数据
        __HAL_UART_CLEAR_FLAG(&huart1, UART_FLAG_RXNE);
    }

    if (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_IDLE) != RESET) // 空闲中断的标志位, 判断uart信息内容并处理
    {
        /*
        uart1 传输层自定协议,在空闲中断里进行格式判定
        +--------+--------+---------+----------+
        |  1byte |  1byte | 252byte |  1byte   |
        +--------+--------+---------+----------+
        | header | length | payload | checksum |
        +--------+--------+---------+----------+
                                |
                                ↓
                    uart1 payload自定协议，在主循环里进行处理
                    +--------+--------+
                    |  2byte | N byte |
                    +--------+--------+
                    |   cmd  | param  |
                    +--------+--------+
                        __|__
                       ↓     ↓
            响应包buffer    指令包buffer
            // 设计双buffer，防止在阻塞等待响应包的过程中，被动接收到指令包无法处理。
        */


        if ((g_uart1_idx > 2) && (g_uart1_rx_buf[0] == 0XAA)) // 检查包长和header
        {
            uint8_t length = g_uart1_rx_buf[1];
            uint16_t cmd = *(uint16_t *)(&g_uart1_rx_buf[2]);
            uint8_t checksum = g_uart1_rx_buf[g_uart1_idx - 1];

            printf("--> uart1 rx %d\n", g_uart1_idx);
            for(int j = 0 ; j < g_uart1_idx; j++ ){
                printf("%02x,", g_uart1_rx_buf[j]);
            }
            printf("\n");

            if (checksum == simple_checksum(g_uart1_rx_buf, g_uart1_idx - 1)) // 检查checksum
            {
                if (cmd == 0x01) // ACK 包
                {
                    if (g_uart1_ack_flag == 0)
                    {
                        memcpy(g_uart1_rx_ack_buf, g_uart1_rx_buf + 2, length); // 拷贝payload到ack buffer
                        g_uart1_cmd_type = 0XAA;
                        g_uart1_ack_len = length;
                        g_uart1_ack_flag = 1;
                    }
                }
                else // CMD包
                {
                    if (g_uart1_cmd_flag == 0)
                    {
                        memcpy(g_uart1_rx_cmd_buf, g_uart1_rx_buf + 2, length); // 拷贝payload到cmd buffer
                        g_uart1_cmd_len = length;
                        g_uart1_cmd_flag = 1;
                    }
                }
            }
            else
            {
                printf("uart1 rx checksum err\n");
            }
        }
        else if ((g_uart1_idx > 2) && (g_uart1_rx_buf[0] == 0XDD))  
        {
          /*
          新版本协议检测和通信，不再检测协议内的ACK，而是通过ACK上下文内是否包含对应cmd和waitackflag，决定将payload拷贝到cmd_buff还是上下文内维护的buff中
            +--------+--------+--------+---------+------------------+
            |  1byte |  1byte |  1byte | 251byte |      1byte       |
            +--------+--------+--------+---------+------------------+
            | header |  cmd   | length | payload | payload_checksum |
            +--------+--------+--------+---------+------------------+
            */  

            // 打印接收到的数据
            print_mem_hexdump("uart1 rx", g_uart1_rx_buf, g_uart1_idx);
            uint8_t cmd = g_uart1_rx_buf[1];
            uint8_t payload_length = g_uart1_rx_buf[2];
            uint8_t *p_payload = g_uart1_rx_buf + 3;
            uint8_t payload_checksum = g_uart1_rx_buf[g_uart1_idx - 1];
            
            if(payload_checksum == simple_checksum(p_payload, payload_length))
            {
              //1.检查是否有ack上下文
              if (g_waitAckCtx.waitAckFlag == 1)
              {
                  // 检查ack上下文内是否包含对应cmd和waitackflag
                  if (g_waitAckCtx.cmd == cmd && g_waitAckCtx.waitAckFlag == 1)
                  {
                      // 将payload拷贝到ack上下文内维护的buff中
                      memcpy(g_waitAckCtx.ackBuf, p_payload, payload_length);
                      g_waitAckCtx.ackLen = payload_length;
                      g_waitAckCtx.ackFlag = 1;
                  }
                  else
                  {
                      printf("uart1 rx ack context not match\n");
                  }
              }
              else
              {
                  // 没有ack上下文，将payload拷贝到cmd_buff中
                  memcpy(g_uart1_rx_cmd_buf, p_payload, payload_length);
                  g_uart1_cmd_cmd = cmd;               // 记录cmd命令
                  g_uart1_cmd_len = payload_length;    // 记录payload长度
                  g_uart1_cmd_type = 0XDD;             // 记录协议类型
                  g_uart1_cmd_flag = 1;                // 设置cmd包接收标志位

                  #if 1
                  print_mem_hexdump("uart1 rx cmd", g_uart1_rx_cmd_buf, g_uart1_cmd_len);
                  #endif
              }
            }
            else
            {
                printf("uart1 rx checksum err\n");
            }
        }
        else
        {
            // 没有header，或者header错误,清空接收buffer
            printf("uart1 rx header err\n");
            print_mem_hexdump("uart1 rx", g_uart1_rx_buf, g_uart1_idx);
        }  

        g_uart1_idx = 0 ;
        __HAL_UART_CLEAR_IDLEFLAG(&huart1); // 清除空闲中断的标志位
    }
}

// NOTE: uart3接收中断异常，暂未深究，功能上暂时不用。
/* uart3 debug ->　mcu 接收中断 */
void USART3_IRQHandler(void)
{
    if (__HAL_UART_GET_FLAG(&huart3, UART_FLAG_RXNE) != RESET) // 串口接收中断
    {
        HAL_UART_Receive(&huart3, &(uart3_rx_buf[g_uart3_idx++]), 1, 100); // 串口2接收1位数据
        __HAL_UART_CLEAR_FLAG(&huart3, UART_FLAG_RXNE);
    }

    if (__HAL_UART_GET_FLAG(&huart3, UART_FLAG_IDLE) != RESET) // 空闲中断,表示一包接收完成
    {
        printf("--> uart3 rx len: %d\n", g_uart3_idx);
        g_uart3_idx = 0;                    // 清除接收buffer索引
        __HAL_UART_CLEAR_IDLEFLAG(&huart3); // 清除空闲中断的标志位
    }
}


#if 0 
/* USER CODE BEGIN 1 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  uint16_t timeout;
  timeout = 0;

  if (huart->Instance == USART1)
  {

    #if 1
    for (int i = 0; i < 18; i++)
    {
      Ethernet_Info[i] = g_uart1_rx_buf[i];
      g_uart1_rx_buf[i] = 0;
    }

    STMFLASH_Write(ADDR_FLASH_SECTOR_6, (uint32_t *)Ethernet_Info, 9);
    Adj_IP();

    // ip_setting_update(&Ethernet_Info[0], &Ethernet_Info[4], &Ethernet_Info[8]);

    HAL_UART_Transmit(&huart1, (uint8_t *)"Ucn", 3, 5000);

    while (HAL_UART_Receive_IT(&huart1, (uint8_t *)g_uart1_rx_buf, 18) != HAL_OK)
    {
      timeout++; //
      if (timeout > 0xFFFF)
      {
        huart->RxState = HAL_UART_STATE_READY;
        __HAL_UNLOCK(&huart1);
        break;
      }
    }

    printf("UpdateIP\n");
    #else

    /* uart接收中断 */
    if(g_uart1_detected == false)
    {
      g_uart1_checkTick = 10;
      g_uart1_rx_buf[g_uart1_idx] = rx_temp;
      g_uart1_idx++; 
      RcvCount++;
    }
    RcvErr = HAL_UART_Receive_IT(&huart1, &rx_temp, 1);
    #endif

  }
  else if (huart->Instance == USART3)
  {
    //NOTE: ??????????????????
    g_uart3_detected = true;
    HAL_UART_Receive_IT(&huart3, (uint8_t *)uart3_rx_buf, 1);
  }
}
#endif

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
  printf(" \r\n --> RcvErr = %d", RcvErr);

#if 0 
  uint16_t timeout;
  timeout = 0;
  if (huart->Instance == USART1)
  {

    if (__HAL_UART_GET_FLAG(huart, UART_FLAG_ORE) != RESET)
    {
      printf("ORE\n");
      __HAL_UART_DISABLE_IT(huart, UART_IT_ERR);
      __HAL_UART_CLEAR_OREFLAG(huart);
    }
    else if (__HAL_UART_GET_FLAG(huart, UART_FLAG_TXE) != RESET)
    {
      printf("TXE\n");
      __HAL_UART_DISABLE_IT(huart, UART_IT_TXE);
      __HAL_UART_DISABLE_IT(huart, UART_IT_TC);
      __HAL_UART_CLEAR_FLAG(huart, UART_FLAG_TXE);
      __HAL_UART_CLEAR_FLAG(huart, UART_FLAG_TC);
    }
    else if (__HAL_UART_GET_FLAG(huart, UART_FLAG_RXNE) != RESET)
    {
      printf("RXNE\n");
      __HAL_UART_DISABLE_IT(huart, UART_IT_RXNE);
      __HAL_UART_CLEAR_FLAG(huart, UART_FLAG_RXNE);
    }
    while (HAL_UART_Receive_IT(&huart1, (uint8_t *)g_uart1_rx_cmd_buf, 18) != HAL_OK)
    {
      timeout++; //
      if (timeout > 0xFFFF)
      {
        huart->RxState = HAL_UART_STATE_READY;
        __HAL_UNLOCK(&huart1);
        break;
      }
    }
  }
  #endif
}

int fputc(int ch, FILE *f)
{
  HAL_UART_Transmit(&huart_debug_x, (uint8_t *)&ch, 1, 0x100);
  return ch;
}

int fgetc(FILE *f)
{
  uint8_t ch = 0;
  HAL_UART_Receive(&huart_debug_x, &ch, 1, 0x100);
  return ch;
}

/* USER CODE END 1 */

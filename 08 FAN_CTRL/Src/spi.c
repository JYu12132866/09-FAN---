/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    spi.c
  * @brief   This file provides code for the configuration
  *          of the SPI instances.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "spi.h"

/* USER CODE BEGIN 0 */
#include <string.h>
#include <stdio.h>

static volatile uint8_t spi2_started = 0;

#define SPI2_MSG_MAX 128U

/* SPI通信状态 */
typedef enum {
  SPI_STATE_IDLE = 0,       /* 等待命令 */
  SPI_STATE_CMD_RX,         /* 接收命令中 */
  SPI_STATE_CMD_READY,      /* 命令接收完成，等待处理 */
  SPI_STATE_RESP_PENDING,   /* 等待设置响应 */
  SPI_STATE_RESP_READY,     /* 响应准备好，可以发送 */
  SPI_STATE_RESP_SENDING,   /* 正在发送响应 */
  SPI_STATE_RESP_SENT       /* 响应发送完成 */
} SPI_State_t;

static volatile SPI_State_t spi_state = SPI_STATE_IDLE;
static volatile uint8_t spi_rx_len = 0;
static volatile uint8_t spi_tx_len = 0;
static volatile uint8_t spi_tx_idx = 0;

static uint8_t spi_rx_buf[SPI2_MSG_MAX];
static uint8_t spi_tx_buf[SPI2_MSG_MAX];

static volatile uint8_t spi_cmd_pending = 0;
static uint8_t spi_cmd_buf[SPI2_MSG_MAX];
static volatile uint8_t spi_cmd_len = 0;

/* USER CODE END 0 */

SPI_HandleTypeDef hspi2;

/* SPI2 init function */
void MX_SPI2_Init(void)
{

  /* USER CODE BEGIN SPI2_Init 0 */

  /* USER CODE END SPI2_Init 0 */

  /* USER CODE BEGIN SPI2_Init 1 */

  /* USER CODE END SPI2_Init 1 */
  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_SLAVE;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi2.Init.NSS = SPI_NSS_HARD_INPUT;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI2_Init 2 */

  /* USER CODE END SPI2_Init 2 */

}

void HAL_SPI_MspInit(SPI_HandleTypeDef* spiHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(spiHandle->Instance==SPI2)
  {
  /* USER CODE BEGIN SPI2_MspInit 0 */

  /* USER CODE END SPI2_MspInit 0 */
    /* SPI2 clock enable */
    __HAL_RCC_SPI2_CLK_ENABLE();

    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**SPI2 GPIO Configuration
    PB12     ------> SPI2_NSS
    PB13     ------> SPI2_SCK
    PB14     ------> SPI2_MISO
    PB15     ------> SPI2_MOSI
    */
    GPIO_InitStruct.Pin = GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_15;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_14;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* SPI2 interrupt Init */
    HAL_NVIC_SetPriority(SPI2_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(SPI2_IRQn);

  /* USER CODE BEGIN SPI2_MspInit 1 */

  /* USER CODE END SPI2_MspInit 1 */
  }
}

void HAL_SPI_MspDeInit(SPI_HandleTypeDef* spiHandle)
{

  if(spiHandle->Instance==SPI2)
  {
  /* USER CODE BEGIN SPI2_MspDeInit 0 */

  /* USER CODE END SPI2_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_SPI2_CLK_DISABLE();

    /**SPI2 GPIO Configuration
    PB12     ------> SPI2_NSS
    PB13     ------> SPI2_SCK
    PB14     ------> SPI2_MISO
    PB15     ------> SPI2_MOSI
    */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15);

    /* SPI2 interrupt Deinit */
    HAL_NVIC_DisableIRQ(SPI2_IRQn);

  /* USER CODE BEGIN SPI2_MspDeInit 1 */

  /* USER CODE END SPI2_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

void SPI2_SlaveComm_Start(void)
{
  if (spi2_started) return;
  spi2_started = 1;
  
  spi_state = SPI_STATE_IDLE;
  spi_rx_len = 0;
  spi_tx_len = 0;
  spi_tx_idx = 0;
  spi_cmd_pending = 0;
  spi_cmd_len = 0;
  
  spi_test = 0;
  spi_test2 = 1;
//16.20
  memset(spi_rx_buf, 0, sizeof(spi_rx_buf));
  memset(spi_tx_buf, 0, sizeof(spi_tx_buf));
  memset(spi_cmd_buf, 0, sizeof(spi_cmd_buf));

  __HAL_SPI_ENABLE(&hspi2);
  
  /* 启用RXNE和ERR中断 */
  SET_BIT(hspi2.Instance->CR2, SPI_CR2_RXNEIE | SPI_CR2_ERRIE);
  
  /* 预加载第一个字节到DR（作为从机，这是为了产生第一个响应） */
  hspi2.Instance->DR = 0x00;
}

uint8_t SPI2_SlaveComm_GetCmd(uint8_t *out, uint16_t maxlen)
{
  if (out == NULL || maxlen == 0) return 0;
  if (!spi_cmd_pending) return 0;
  
  uint16_t len = spi_cmd_len;
  if (len > maxlen - 1) len = maxlen - 1;
  
  memcpy(out, spi_cmd_buf, len);
  out[len] = '\0';
  
  spi_cmd_pending = 0;
  
  return (uint8_t)len;
}

void SPI2_SlaveComm_SetResponse(const uint8_t *data, uint16_t len)
{
  if (data == NULL || len == 0) {
    spi_tx_len = 0;
    return;
  }
  
  if (len > SPI2_MSG_MAX) len = SPI2_MSG_MAX;
  
  /* 复制响应数据 */
  memcpy(spi_tx_buf, data, len);
  spi_tx_len = len;
  spi_tx_idx = 0;
  
  /* 标记响应已准备好 */
  spi_state = SPI_STATE_RESP_READY;
}

uint8_t SPI2_SlaveComm_HasPendingCmd(void)
{
  return spi_cmd_pending;
}

uint8_t SPI2_SlaveComm_IsResponseReady(void)
{
  return (spi_state == SPI_STATE_RESP_READY);
}

/* SPI中断处理 - 简化的协议：
 * 1. 主MCU发送命令字节，从MCU接收并存入缓冲区
 * 2. 当收到\r\n时，命令接收完成，标记为待处理
 * 3. 从MCU处理命令并设置响应
 * 4. 主MCU发送dummy字节读取响应，从MCU从缓冲区发送
 */
void SPI2_SlaveIRQHandler(void)
{
  SPI_TypeDef *spi = hspi2.Instance;
  uint16_t sr = spi->SR;
  
  /* 处理接收 */
  if ((sr & SPI_SR_RXNE) != 0U) {
    uint8_t rx_byte = (uint8_t)spi->DR;
    
    switch (spi_state) {
      case SPI_STATE_IDLE:
      case SPI_STATE_CMD_RX:
        /* 接收命令字节 */
        if (rx_byte == '\r') {
          /* 忽略\r */
        } else if (rx_byte == '\n') {
          /* 命令结束 */
          if (spi_rx_len > 0) {
            memcpy(spi_cmd_buf, spi_rx_buf, spi_rx_len);
            spi_cmd_buf[spi_rx_len] = '\0';
            spi_cmd_len = spi_rx_len;
            spi_cmd_pending = 1;
            spi_state = SPI_STATE_CMD_READY;
          }
          spi_rx_len = 0;
        } else if (spi_rx_len < SPI2_MSG_MAX) {
          spi_rx_buf[spi_rx_len++] = rx_byte;
          spi_state = SPI_STATE_CMD_RX;
        }
        break;
        
      case SPI_STATE_RESP_READY:
      case SPI_STATE_RESP_SENDING:
        /* 在响应阶段，接收到的字节是dummy，忽略 */
        break;
        
      default:
        break;
    }
  }
  
  /* 处理发送 */
  sr = spi->SR;
  if ((sr & SPI_SR_TXE) != 0U) {
    uint8_t tx_byte = 0x00;
    
    if (spi_state == SPI_STATE_RESP_READY || spi_state == SPI_STATE_RESP_SENDING) {
      if (spi_tx_idx < spi_tx_len) {
        tx_byte = spi_tx_buf[spi_tx_idx++];
        spi_state = SPI_STATE_RESP_SENDING;
      } else {
        /* 响应发送完成 */
        tx_byte = 0x00;
        spi_state = SPI_STATE_RESP_SENT;
      }
    }
    
    spi->DR = tx_byte;
  }
  
  /* 处理错误 */
  sr = spi->SR;
  if ((sr & SPI_SR_OVR) != 0U) {
    /* 清除溢出错误 */
    (void)spi->DR;
    (void)spi->SR;
    /* 重置状态 */
    if (spi_state != SPI_STATE_RESP_READY && spi_state != SPI_STATE_RESP_SENDING) {
      spi_state = SPI_STATE_IDLE;
      spi_rx_len = 0;
    }
  }
  
  if ((sr & SPI_SR_MODF) != 0U) {
    __HAL_SPI_CLEAR_MODFFLAG(&hspi2);
    if (spi_state != SPI_STATE_RESP_READY && spi_state != SPI_STATE_RESP_SENDING) {
      spi_state = SPI_STATE_IDLE;
      spi_rx_len = 0;
    }
  }
}

/* 轮询NSS状态，检测通信结束 */
void SPI2_SlaveComm_Poll(void)
{
  static uint8_t last_nss = 1;
  uint8_t current_nss = (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_12) == GPIO_PIN_SET) ? 1 : 0;
  
  /* NSS上升沿 - 通信结束 */
  if (last_nss == 0 && current_nss == 1) {
    /* 如果正在发送响应，标记为完成 */
    if (spi_state == SPI_STATE_RESP_SENDING || spi_state == SPI_STATE_RESP_SENT) {
      spi_state = SPI_STATE_IDLE;
      spi_tx_idx = 0;
    }
    /* 如果在接收命令过程中被中断，重置 */
    if (spi_state == SPI_STATE_CMD_RX) {
      spi_state = SPI_STATE_IDLE;
      spi_rx_len = 0;
    }
  }
  
  last_nss = current_nss;
}

HAL_StatusTypeDef HAL_SPI_Enable(SPI_HandleTypeDef *hspi)
{
  if (hspi == NULL) return HAL_ERROR;
  __HAL_SPI_ENABLE(hspi);
  return HAL_OK;
}

/* USER CODE BEGIN 1 */

/* 命令处理回调函数指针 */
static SPI_CommandHandler_t cmd_handler = NULL;

/**
  * @brief  注册SPI命令处理函数
  * @param  handler: 命令处理函数指针
  * @retval None
  */
void SPI2_RegisterCommandHandler(SPI_CommandHandler_t handler)
{
    cmd_handler = handler;
}

/**
  * @brief  处理SPI命令（在main循环中调用）
  * @retval None
  */
void SPI2_Process(void)
{
    if (!SPI2_SlaveComm_HasPendingCmd())
    {
        return;
    }
    
    uint8_t spi_cmd[SPI2_MSG_MAX];
    uint8_t spi_cmd_len = SPI2_SlaveComm_GetCmd(spi_cmd, sizeof(spi_cmd));
    
    if (spi_cmd_len == 0)
    {
        return;
    }
    
    /* 如果有注册的处理函数，调用它 */
    if (cmd_handler != NULL)
    {
        uint8_t response_buf[SPI2_MSG_MAX];
        uint8_t response_len = 0;
        
        cmd_handler(spi_cmd, spi_cmd_len, response_buf, &response_len, sizeof(response_buf));
        
        if (response_len > 0)
        {
            SPI2_SlaveComm_SetResponse(response_buf, response_len);
        }
    }
}

/* USER CODE END 1 */

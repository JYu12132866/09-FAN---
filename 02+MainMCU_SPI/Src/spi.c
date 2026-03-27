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
#include "wwdg.h"  // 添加看门狗头文件
#include <string.h>  // 添加string.h以使用memset

/* USER CODE BEGIN 0 */

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
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_64;
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
    /* SPI2 NSS (CS) - PB12 - 作为GPIO输出，软件控制 */
    GPIO_InitStruct.Pin = GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET); // 默认片选高电平（无效）

    /* SPI2 SCK - PB13 - 复用推挽输出 */
    GPIO_InitStruct.Pin = GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* SPI2 MISO - PB14 - 输入模式 */
    GPIO_InitStruct.Pin = GPIO_PIN_14;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* SPI2 MOSI - PB15 - 复用推挽输出 */
    GPIO_InitStruct.Pin = GPIO_PIN_15;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

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
    HAL_GPIO_DeInit(GPIOB, SPI1_NSS_Pin|SPI1_SCK_Pin|SPI1_MISO_Pin|SPI1_MOSI_Pin);

  /* USER CODE BEGIN SPI2_MspDeInit 1 */

  /* USER CODE END SPI2_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

/**
  * @brief  SPI2传输数据
  * @param  tx_data: 要发送的数据
  * @param  rx_data: 接收数据缓冲区
  * @param  size: 数据大小
  * @retval HAL状态
  */
HAL_StatusTypeDef SPI2_TransmitReceive(uint8_t *tx_data, uint8_t *rx_data, uint16_t size)
{
    HAL_StatusTypeDef status;
    
    // 确保SPI不在忙状态
    if (hspi2.State == HAL_SPI_STATE_BUSY_TX || hspi2.State == HAL_SPI_STATE_BUSY_RX) {
        HAL_SPI_Abort(&hspi2);
        for (volatile int i = 0; i < 1000; i++) {
            HAL_WWDG_Refresh(&hwwdg);
        }
    }
    
    // 清空接收缓冲区
    memset(rx_data, 0, size);
    
    // 选择设备（片选低电平）
    HAL_GPIO_WritePin(SPI2_CS_PORT, SPI2_CS_PIN, GPIO_PIN_RESET);
    
    // 短暂延时，确保从设备准备好
    for (volatile int i = 0; i < 1000; i++) {
        HAL_WWDG_Refresh(&hwwdg);
    }
    
    // 发送/接收数据
    status = HAL_SPI_TransmitReceive(&hspi2, tx_data, rx_data, size, 100);
    
    // 短暂延时，确保数据传输完成
    for (volatile int i = 0; i < 1000; i++) {
        HAL_WWDG_Refresh(&hwwdg);
    }
    
    // 取消选择设备（片选高电平）
    HAL_GPIO_WritePin(SPI2_CS_PORT, SPI2_CS_PIN, GPIO_PIN_SET);
    
    return status;
}

HAL_StatusTypeDef SPI2_TransmitThenReceive(uint8_t *tx_data, uint16_t tx_size,
                                          uint8_t *rx_data, uint16_t rx_size)
{
    HAL_StatusTypeDef status = HAL_OK;

    // 确保SPI不在忙状态
    if (hspi2.State == HAL_SPI_STATE_BUSY_TX || hspi2.State == HAL_SPI_STATE_BUSY_RX) {
        HAL_SPI_Abort(&hspi2);
        for (volatile int i = 0; i < 1000; i++) {
            HAL_WWDG_Refresh(&hwwdg);
        }
    }

    // 清空接收缓冲区
    if (rx_data != NULL && rx_size > 0) {
        memset(rx_data, 0, rx_size);
    }

    // 选择设备（片选低电平）
    HAL_GPIO_WritePin(SPI2_CS_PORT, SPI2_CS_PIN, GPIO_PIN_RESET);

    // 短暂延时，确保从设备准备好
    for (volatile int i = 0; i < 1000; i++) {
        HAL_WWDG_Refresh(&hwwdg);
    }

    // 先发送命令/数据
    if (tx_data != NULL && tx_size > 0) {
        status = HAL_SPI_Transmit(&hspi2, tx_data, tx_size, 100);
        if (status != HAL_OK) {
            HAL_GPIO_WritePin(SPI2_CS_PORT, SPI2_CS_PIN, GPIO_PIN_SET);
            return status;
        }
    }

    // 重要：给从MCU足够的时间处理命令并准备响应
    // 从MCU需要时间来解析命令、执行操作（如转发到UART）、获取响应
    // 普通命令延时约50ms，FGRD命令需要更长时间（约500ms）等待FG捕获
    uint32_t delay_target = 50000; // 默认50ms
    
    // 检查是否是FGRD命令（以FGRD开头）
    if (tx_data != NULL && tx_size >= 4) {
        if ((tx_data[0] == 'F' || tx_data[0] == 'f') &&
            (tx_data[1] == 'G' || tx_data[1] == 'g') &&
            (tx_data[2] == 'R' || tx_data[2] == 'r') &&
            (tx_data[3] == 'D' || tx_data[3] == 'd')) {
            delay_target = 500000; // FGRD命令延时约500ms
        }
    }
    
    for (volatile uint32_t delay_cnt = 0; delay_cnt < delay_target; delay_cnt++) {
        HAL_WWDG_Refresh(&hwwdg);
    }

    // 接收响应 - 使用全双工传输接收数据
    if (rx_data != NULL && rx_size > 0) {
        // 使用0xFF作为dummy字节
        uint8_t dummy[32];
        memset(dummy, 0xFF, sizeof(dummy));

        uint16_t remaining = rx_size;
        uint16_t offset = 0;
        while (remaining > 0) {
            uint16_t chunk = (remaining > (uint16_t)sizeof(dummy)) ? (uint16_t)sizeof(dummy) : remaining;
            status = HAL_SPI_TransmitReceive(&hspi2, dummy, &rx_data[offset], chunk, 100);
            if (status != HAL_OK) {
                HAL_GPIO_WritePin(SPI2_CS_PORT, SPI2_CS_PIN, GPIO_PIN_SET);
                return status;
            }
            remaining -= chunk;
            offset += chunk;
        }
    }

    // 短暂延时，确保总线空闲（期间喂狗）
    for (volatile int i = 0; i < 1000; i++) {
        HAL_WWDG_Refresh(&hwwdg);
    }

    // 取消选择设备（片选高电平）
    HAL_GPIO_WritePin(SPI2_CS_PORT, SPI2_CS_PIN, GPIO_PIN_SET);

    return status;
}

/**
  * @brief  SPI2发送数据
  * @param  data: 要发送的数据
  * @param  size: 数据大小
  * @retval HAL状态
  */
HAL_StatusTypeDef SPI2_Transmit(uint8_t *data, uint16_t size)
{
    HAL_StatusTypeDef status;
    
    // 确保SPI不再忙状态
    if (hspi2.State == HAL_SPI_STATE_BUSY_TX || hspi2.State == HAL_SPI_STATE_BUSY_RX) {
        // 尝试重置SPI状态
        HAL_SPI_Abort(&hspi2);
        HAL_Delay(1);
    }
    
    // 选择设备（片选低电平）
    HAL_GPIO_WritePin(SPI2_CS_PORT, SPI2_CS_PIN, GPIO_PIN_RESET);
    
    // 短暂延时，确保从设备准备好
    // 注意：在延时期间需要喂狗
    for (volatile int i = 0; i < 1000; i++) {
        HAL_WWDG_Refresh(&hwwdg);
    }
    
    // 发送数据 - 减少超时时间，避免看门狗复位
    // 7个字符 × 8位/字符 ÷ 500kHz ≈ 112μs，设置100ms超时足够
    status = HAL_SPI_Transmit(&hspi2, data, size, 100); // 100ms超时
    
    // 取消选择设备（片选高电平）
    HAL_GPIO_WritePin(SPI2_CS_PORT, SPI2_CS_PIN, GPIO_PIN_SET);
    
    return status;
}

/**
  * @brief  选择SPI设备
  * @param  select: 1-选择设备（低电平），0-取消选择（高电平）
  * @retval None
  */
void SPI2_SelectDevice(uint8_t select)
{
    if (select) {
        HAL_GPIO_WritePin(SPI2_CS_PORT, SPI2_CS_PIN, GPIO_PIN_RESET); // 选择设备
    } else {
        HAL_GPIO_WritePin(SPI2_CS_PORT, SPI2_CS_PIN, GPIO_PIN_SET);   // 取消选择
    }
}

/* USER CODE END 1 */

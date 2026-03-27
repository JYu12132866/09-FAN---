/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    i2c.c
  * @brief   This file provides code for the configuration
  *          of the I2C instances.
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
#include "i2c.h"
#include "usart.h"
#include <stdio.h>

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

I2C_HandleTypeDef hi2c;

/* I2C init function */
void MX_I2C_Init(void)
{

  /* USER CODE BEGIN I2C_Init 0 */
  UART_SendString("[DEBUG] Initializing I2C...\r\n");
  /* USER CODE END I2C_Init 0 */

  /* USER CODE BEGIN I2C_Init 1 */

  /* USER CODE END I2C_Init 1 */
  hi2c.Instance = I2C2;
  hi2c.Init.ClockSpeed = 50000; // 降低I2C2时钟速度以提高稳定性
  hi2c.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c.Init.OwnAddress1 = 0;
  hi2c.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c.Init.OwnAddress2 = 0;
  hi2c.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  
  char debug_buf[64];
  sprintf(debug_buf, "[DEBUG] I2C ClockSpeed: %d Hz\r\n", hi2c.Init.ClockSpeed);
  UART_SendString(debug_buf);
  
  if (HAL_I2C_Init(&hi2c) != HAL_OK)
  {
    UART_SendString("[DEBUG] I2C initialization failed\r\n");
    Error_Handler();
  }
  
  UART_SendString("[DEBUG] I2C initialization successful\r\n");
  
  // 检查I2C状态
  HAL_I2C_StateTypeDef state = HAL_I2C_GetState(&hi2c);
  sprintf(debug_buf, "[DEBUG] I2C State after init: %d\r\n", state);
  UART_SendString(debug_buf);
  
  /* USER CODE BEGIN I2C_Init 2 */

  /* USER CODE END I2C_Init 2 */

}

void HAL_I2C_MspInit(I2C_HandleTypeDef* i2cHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(i2cHandle->Instance==I2C2)
  {
  /* USER CODE BEGIN I2C_MspInit 0 */
  UART_SendString("[DEBUG] I2C MspInit started\r\n");
  /* USER CODE END I2C_MspInit 0 */

    __HAL_RCC_GPIOB_CLK_ENABLE();
    UART_SendString("[DEBUG] GPIOB clock enabled\r\n");
    
    /**I2C GPIO Configuration
    PB10     ------> I2C_SCL
    PB11     ------> I2C_SDA
    */
    GPIO_InitStruct.Pin = GPIO_PIN_10|GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    UART_SendString("[DEBUG] I2C GPIO initialized (PB10: SCL, PB11: SDA)\r\n");

    /* I2C clock enable */
    __HAL_RCC_I2C2_CLK_ENABLE();
    UART_SendString("[DEBUG] I2C clock enabled\r\n");

    /* I2C interrupt Init */
    HAL_NVIC_SetPriority(I2C2_EV_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(I2C2_EV_IRQn);
    HAL_NVIC_SetPriority(I2C2_ER_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(I2C2_ER_IRQn);
    UART_SendString("[DEBUG] I2C interrupts enabled\r\n");
  /* USER CODE BEGIN I2C_MspInit 1 */

  /* USER CODE END I2C_MspInit 1 */
  }
}

void HAL_I2C_MspDeInit(I2C_HandleTypeDef* i2cHandle)
{

  if(i2cHandle->Instance==I2C2)
  {
  /* USER CODE BEGIN I2C_MspDeInit 0 */

  /* USER CODE END I2C_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_I2C2_CLK_DISABLE();

    /**I2C GPIO Configuration
    PB10     ------> I2C_SCL
    PB11     ------> I2C_SDA
    */
    HAL_GPIO_DeInit(I2C_SCL_ADC_GPIO_Port, I2C_SCL_ADC_Pin);

    HAL_GPIO_DeInit(I2C_SCL_ADCB11_GPIO_Port, I2C_SCL_ADCB11_Pin);

    /* I2C interrupt Deinit */
    HAL_NVIC_DisableIRQ(I2C2_EV_IRQn);
    HAL_NVIC_DisableIRQ(I2C2_ER_IRQn);
  /* USER CODE BEGIN I2C_MspDeInit 1 */

  /* USER CODE END I2C_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

// TLA2528相关函数实现

/**
 * @brief  向TLA2528写入单个寄存器（带opc code）
 * @param  reg_addr: 寄存器地址
 * @param  data: 要写入的数据
 * @retval HAL_StatusTypeDef: 操作状态
 */
HAL_StatusTypeDef TLA2528_WriteReg(uint8_t reg_addr, uint8_t data)
{
    // TLA2528单寄存器写入的opc code是0x08（根据手册7.5.2.1）
    // 构建正确的写入序列：[opcode, reg_addr, data]
    uint8_t tx_data[3] = {0x08, reg_addr, data};
    
    // 尝试多次写入
    for (uint8_t i = 0; i < 3; i++)
    {
        // 确保I2C总线空闲
        while (HAL_I2C_GetState(&hi2c) != HAL_I2C_STATE_READY) {
            HAL_Delay(1);
        }
        
        // 发送数据
        HAL_StatusTypeDef status = HAL_I2C_Master_Transmit(&hi2c, TLA2528_I2C_WRITE_ADDR, tx_data, 3, 100);
        
        if (status == HAL_OK)
        {
            // 添加延时确保写入完成
            HAL_Delay(20); // 增加延时时间，确保写入操作完全完成
            return status;
        }
        HAL_Delay(5);
    }
    
    return HAL_ERROR;
}

/**
 * @brief  从TLA2528读取单个寄存器（带opc code）
 * @param  reg_addr: 寄存器地址
 * @param  data: 读取的数据存储地址
 * @retval HAL_StatusTypeDef: 操作状态
 */
HAL_StatusTypeDef TLA2528_ReadReg(uint8_t reg_addr, uint8_t *data)
{
    // TLA2528单寄存器读取的opc code是0x10（根据手册）
    // 构建正确的命令序列：[opcode, reg_addr]
    uint8_t tx_data[2] = {0x10, reg_addr};
    
    // 尝试多次读取
    for (uint8_t i = 0; i < 3; i++)
    {
        // 确保I2C总线空闲
        while (HAL_I2C_GetState(&hi2c) != HAL_I2C_STATE_READY) {
            HAL_Delay(1);
        }
        
        // 发送命令
        HAL_StatusTypeDef tx_status = HAL_I2C_Master_Transmit(&hi2c, TLA2528_I2C_WRITE_ADDR, tx_data, 2, 100);
        if (tx_status != HAL_OK)
        {
            HAL_Delay(5);
            continue;
        }
        
        // 确保I2C总线空闲
        while (HAL_I2C_GetState(&hi2c) != HAL_I2C_STATE_READY) {
            HAL_Delay(1);
        }
        
        // 读取数据
        HAL_StatusTypeDef rx_status = HAL_I2C_Master_Receive(&hi2c, TLA2528_I2C_READ_ADDR, data, 1, 100);
        if (rx_status == HAL_OK)
        {
            return HAL_OK;
        }
        HAL_Delay(5);
    }
    
    return HAL_ERROR;
}

/**
 * @brief  直接读取TLA2528的ADC转换结果
 * @param  value: 读取的12位ADC值
 * @param  raw_bytes: 原始2字节数据（可选，传NULL则不返回）
 * @retval HAL_StatusTypeDef: 操作状态
 */
HAL_StatusTypeDef TLA2528_ReadADC(uint16_t *value, uint8_t *raw_bytes)
{
    uint8_t result[2];

    // 根据数据手册 7.3.9 和图 6，转换完成后，主机只需对器件发起一次
    // 2 字节的 I2C 读操作即可获得 12bit 转换结果：
    // 第一个字节：D11..D4
    // 第二个字节：D3..D0 位于高 4 位，低 4 位为状态或填充位
    if (HAL_I2C_Master_Receive(&hi2c, TLA2528_I2C_READ_ADDR, result, 2, HAL_MAX_DELAY) != HAL_OK)
    {
        return HAL_ERROR;
    }

    // 保存原始字节数据
    if (raw_bytes != NULL)
    {
        raw_bytes[0] = result[0];
        raw_bytes[1] = result[1];
    }

    // 组合 12 位 ADC 值
    *value = ((uint16_t)result[0] << 4) | ((result[1] >> 4) & 0x0F);

    return HAL_OK;
}

/**
 * @brief  触发TLA2528单次转换
 * @retval HAL_StatusTypeDef: 操作状态
 */
HAL_StatusTypeDef TLA2528_TriggerConversion(void)
{
    // 使用 GENERAL_CFG.CNVST 位启动一次转换（bit3，参考 7.6.2）
    return TLA2528_WriteReg(TLA2528_REG_GENERAL_CFG, TLA2528_CMD_SINGLE_CONV);
}

/**
 * @brief  读取TLA2528指定通道的转换结果
 * @param  channel: 通道号 (0-7)
 * @param  value: 读取的12位ADC值
 * @param  raw_bytes: 原始2字节数据（可选，传NULL则不返回）
 * @retval HAL_StatusTypeDef: 操作状态
 */
HAL_StatusTypeDef TLA2528_ReadChannel(uint8_t channel, uint16_t *value, uint8_t *raw_bytes)
{
    if (channel > 7)
    {
        return HAL_ERROR;
    }
    
    // 先选择通道
    if (TLA2528_WriteReg(TLA2528_REG_CHANNEL_SEL, channel) != HAL_OK) // 0x11是CHANNEL_SEL寄存器
    {
        return HAL_ERROR;
    }
    
    // 触发转换
    if (TLA2528_TriggerConversion() != HAL_OK)
    {
        return HAL_ERROR;
    }
    
    // 短暂延时
    HAL_Delay(1);
    
    // 直接读取ADC结果
    return TLA2528_ReadADC(value, raw_bytes);
}

/* USER CODE END 1 */

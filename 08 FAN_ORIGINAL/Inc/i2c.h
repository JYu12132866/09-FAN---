/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    i2c.h
  * @brief   This file contains all the function prototypes for
  *          the i2c.c file
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __I2C_H__
#define __I2C_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* USER CODE BEGIN Private defines */

// UART相关函数声明
void UART_SendString(const char *msg);
void UART_Printf(const char *format, ...);

/* USER CODE END Private defines */

extern I2C_HandleTypeDef hi2c;

/* USER CODE BEGIN Private defines */

// TLA2528相关定义
// 参考手册 Table 2：ADDR 引脚设置为得到 0x10 这一 7bit 器件地址
#define TLA2528_I2C_ADDR          0x10      // 7位I2C地址 (0010_000b)
#define TLA2528_I2C_WRITE_ADDR    (TLA2528_I2C_ADDR << 1)       // 写操作地址：0x20
#define TLA2528_I2C_READ_ADDR     ((TLA2528_I2C_ADDR << 1) | 1) // 读操作地址：0x21

// TLA2528寄存器地址（按照手册定义）
#define TLA2528_REG_SYSTEM_STATUS    0x00      // 系统状态寄存器 (reset = 0x80)
#define TLA2528_REG_GENERAL_CFG      0x01      // 通用配置寄存器 (reset = 0x00)
#define TLA2528_REG_DATA_CFG         0x02      // 数据配置寄存器 (reset = 0x00)
#define TLA2528_REG_OSR_CFG          0x03      // 过采样配置寄存器 (reset = 0x00)
#define TLA2528_REG_OPMODE_CFG       0x04      // 操作模式配置寄存器 (reset = 0x00)
#define TLA2528_REG_PIN_CFG          0x05      // 引脚配置寄存器低位 (reset = 0x00, 控制AIN0-3)
#define TLA2528_REG_PIN_CFG_HIGH     0x06      // 引脚配置寄存器高位 (reset = 0x00, 控制AIN4-7)
#define TLA2528_REG_GPIO_CFG         0x07      // GPIO配置寄存器 (reset = 0x00)
#define TLA2528_REG_GPO_DRIVE_CFG    0x09      // GPO驱动配置寄存器 (reset = 0x00)
#define TLA2528_REG_GPO_OUTPUT_VAL   0x0B      // GPO输出值寄存器 (reset = 0x00)
#define TLA2528_REG_GPI_VALUE_LSB    0x0D      // GPI值LSB寄存器 (reset = 0x00)
#define TLA2528_REG_SEQUENCE_CFG     0x10      // 序列配置寄存器 (reset = 0x00)
#define TLA2528_REG_CHANNEL_SEL      0x11      // 通道选择寄存器 (reset = 0x00)
#define TLA2528_REG_AUTO_SEQ_CHSEL   0x12      // 自动序列通道选择寄存器 (reset = 0x00)

// 兼容旧定义
#define TLA2528_REG_CONFIG           TLA2528_REG_SYSTEM_STATUS
#define TLA2528_REG_OS_CMD           TLA2528_REG_GENERAL_CFG

// 命令定义（参考 GENERAL_CFG 寄存器描述，CNVST = bit3）
// 写 GENERAL_CFG=0x08 表示将 CNVST 位置 1，启动一次转换，读回始终为 0
#define TLA2528_CMD_SINGLE_CONV      0x08      // 触发单次转换（CNVST bit）

/* USER CODE END Private defines */

void MX_I2C_Init(void);

/* USER CODE BEGIN Prototypes */

// TLA2528相关函数
HAL_StatusTypeDef TLA2528_WriteReg(uint8_t reg_addr, uint8_t data);
HAL_StatusTypeDef TLA2528_ReadReg(uint8_t reg_addr, uint8_t *data);
HAL_StatusTypeDef TLA2528_ReadMultiReg(uint8_t reg_addr, uint8_t *data, uint8_t len);
HAL_StatusTypeDef TLA2528_ReadADC(uint16_t *value, uint8_t *raw_bytes);
HAL_StatusTypeDef TLA2528_TriggerConversion(void);
HAL_StatusTypeDef TLA2528_ReadChannel(uint8_t channel, uint16_t *value, uint8_t *raw_bytes);

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __I2C_H__ */


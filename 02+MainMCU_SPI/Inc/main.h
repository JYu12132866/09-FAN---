/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define OSCIN_Pin GPIO_PIN_0
#define OSCIN_GPIO_Port GPIOD
#define OSCOUT_Pin GPIO_PIN_1
#define OSCOUT_GPIO_Port GPIOD
#define WKUP_Pin GPIO_PIN_0
#define WKUP_GPIO_Port GPIOA
#define TIM2_CH0_FG_Pin GPIO_PIN_6
#define TIM2_CH0_FG_GPIO_Port GPIOA
#define LED2_Pin GPIO_PIN_5
#define LED2_GPIO_Port GPIOC
#define I2C1_SCL_ADC_Pin GPIO_PIN_10
#define I2C1_SCL_ADC_GPIO_Port GPIOB
#define I2C1_SCL_ADCB11_Pin GPIO_PIN_11
#define I2C1_SCL_ADCB11_GPIO_Port GPIOB
#define SPI1_NSS_Pin GPIO_PIN_12
#define SPI1_NSS_GPIO_Port GPIOB
#define SPI1_SCK_Pin GPIO_PIN_13
#define SPI1_SCK_GPIO_Port GPIOB
#define SPI1_MISO_Pin GPIO_PIN_14
#define SPI1_MISO_GPIO_Port GPIOB
#define SPI1_MOSI_Pin GPIO_PIN_15
#define SPI1_MOSI_GPIO_Port GPIOB
#define TIM7_CH0_PWM_PUMP_Pin GPIO_PIN_6
#define TIM7_CH0_PWM_PUMP_GPIO_Port GPIOC
#define TIM7_CH1_PWM_FAN1_Pin GPIO_PIN_7
#define TIM7_CH1_PWM_FAN1_GPIO_Port GPIOC
#define TIM7_CH2_PWM_FAN2_Pin GPIO_PIN_8
#define TIM7_CH2_PWM_FAN2_GPIO_Port GPIOC
#define TIM7_CH3_PWM_FAN3_Pin GPIO_PIN_9
#define TIM7_CH3_PWM_FAN3_GPIO_Port GPIOC
#define MUX_EN0_Pin GPIO_PIN_11
#define MUX_EN0_GPIO_Port GPIOA
#define MUX_EN1_Pin GPIO_PIN_12
#define MUX_EN1_GPIO_Port GPIOA
#define SWDIO_Pin GPIO_PIN_13
#define SWDIO_GPIO_Port GPIOA
#define SWCLK_Pin GPIO_PIN_14
#define SWCLK_GPIO_Port GPIOA
#define LED1_Pin GPIO_PIN_15
#define LED1_GPIO_Port GPIOA
#define UART1_TX_Pin GPIO_PIN_9
#define UART1_TX_GPIO_Port GPIOA
#define UART1_RX_Pin GPIO_PIN_10
#define UART1_RX_GPIO_Port GPIOA
#define MUX_ADDR0_Pin GPIO_PIN_3
#define MUX_ADDR0_GPIO_Port GPIOB
#define MUX_ADDR1_Pin GPIO_PIN_4
#define MUX_ADDR1_GPIO_Port GPIOB
#define MUX_ADDR2_Pin GPIO_PIN_5
#define MUX_ADDR2_GPIO_Port GPIOB
#define I2C0_SCL_TEMP_Pin GPIO_PIN_6
#define I2C0_SCL_TEMP_GPIO_Port GPIOB
#define I2C0_SDA_TEMP_Pin GPIO_PIN_7
#define I2C0_SDA_TEMP_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
#include "main.h"
#include "i2c.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "wwdg.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>
#include <ctype.h>
#include "fanctrl.h"
#include "led.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
static volatile uint32_t wwdg_counter = 0;

/* 模块状态标志 */
static uint8_t ntc_stream_enable = 0;
static uint8_t fg_frq_enable = 0;

/* 定时器 */
static uint32_t last_ntc_tick = 0;
static uint32_t last_fg_tick = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* 命令处理函数声明 */
static void Main_SPI_CommandHandler(uint8_t *cmd, uint8_t cmd_len, 
                                     uint8_t *response, uint8_t *response_len, 
                                     uint16_t response_maxlen);
static void Main_USART1_CommandHandler(uint8_t *cmd, uint8_t cmd_len);
static void Main_USART2_CommandHandler(uint8_t *cmd, uint8_t cmd_len);

/* 辅助函数 */
static void trim_cmd(char *s);
static int equals_ignore_case(const char *a, const char *b);
static uint8_t parse_xpwm_cmd(const char *cmd, uint8_t *out_pwm_id, uint8_t *out_off_percent);
static uint8_t parse_fgrd_cmd(const char *cmd,
                              uint8_t *out_en0, uint8_t *out_en1,
                              uint8_t *out_addr0, uint8_t *out_addr1, uint8_t *out_addr2);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

static void trim_cmd(char *s)
{
    if (s == NULL) return;

    /* 去掉尾部空白 */
    size_t n = strlen(s);
    while (n > 0 && (s[n - 1] == ' ' || s[n - 1] == '\t' || s[n - 1] == '\n' || s[n - 1] == '\r')) {
        s[n - 1] = '\0';
        n--;
    }

    /* 去掉头部空白：左移 */
    size_t i = 0;
    while (s[i] == ' ' || s[i] == '\t' || s[i] == '\n' || s[i] == '\r') {
        i++;
    }
    if (i > 0) {
        memmove(s, s + i, strlen(s + i) + 1);
    }
}

static int equals_ignore_case(const char *a, const char *b)
{
    if (a == NULL || b == NULL) return 0;
    while (*a && *b)
    {
        if (toupper((unsigned char)*a) != toupper((unsigned char)*b)) return 0;
        a++;
        b++;
    }
    return (*a == '\0' && *b == '\0');
}

static uint8_t parse_xpwm_cmd(const char *cmd, uint8_t *out_pwm_id, uint8_t *out_off_percent)
{
    if (cmd == NULL || out_pwm_id == NULL || out_off_percent == NULL) return 0;
    if (strlen(cmd) != 7) return 0;

    if (!isdigit((unsigned char)cmd[0])) return 0;
    uint8_t pwm_id = (uint8_t)(cmd[0] - '0');
    if (pwm_id > 3) return 0;

    if (toupper((unsigned char)cmd[1]) != 'P' ||
        toupper((unsigned char)cmd[2]) != 'W' ||
        toupper((unsigned char)cmd[3]) != 'M') {
        return 0;
    }

    if (!isdigit((unsigned char)cmd[4]) ||
        !isdigit((unsigned char)cmd[5]) ||
        !isdigit((unsigned char)cmd[6])) {
        return 0;
    }

    uint16_t off = (uint16_t)(cmd[4] - '0') * 100U +
                    (uint16_t)(cmd[5] - '0') * 10U +
                    (uint16_t)(cmd[6] - '0');
    if (off > 100U) return 0;

    *out_pwm_id = pwm_id;
    *out_off_percent = (uint8_t)off;
    return 1;
}

static uint8_t parse_fgrd_cmd(const char *cmd,
                              uint8_t *out_en0, uint8_t *out_en1,
                              uint8_t *out_addr0, uint8_t *out_addr1, uint8_t *out_addr2)
{
    if (cmd == NULL ||
        out_en0 == NULL || out_en1 == NULL ||
        out_addr0 == NULL || out_addr1 == NULL || out_addr2 == NULL) {
        return 0;
    }

    if (strlen(cmd) != 8) return 0;

    if (toupper((unsigned char)cmd[0]) != 'F' ||
        toupper((unsigned char)cmd[1]) != 'G' ||
        toupper((unsigned char)cmd[2]) != 'R' ||
        toupper((unsigned char)cmd[3]) != 'D') {
        return 0;
    }

    if (!isdigit((unsigned char)cmd[4]) ||
        !isdigit((unsigned char)cmd[5]) ||
        !isdigit((unsigned char)cmd[6]) ||
        !isdigit((unsigned char)cmd[7])) {
        return 0;
    }

    if (cmd[4] != '0' && cmd[4] != '1') return 0;
    if (cmd[5] != '0' && cmd[5] != '1') return 0;
    if (cmd[6] != '0' && cmd[6] != '1') return 0;
    if (cmd[7] != '0' && cmd[7] != '1') return 0;

    if (cmd[4] == '0') {
        *out_en0 = 0;
        *out_en1 = 1;
    } else {
        *out_en0 = 1;
        *out_en1 = 0;
    }

    *out_addr0 = (uint8_t)(cmd[5] - '0');
    *out_addr1 = (uint8_t)(cmd[6] - '0');
    *out_addr2 = (uint8_t)(cmd[7] - '0');
    return 1;
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  MX_TIM4_Init();
  MX_TIM3_Init();
  MX_TIM2_Init();
  MX_I2C_Init();
  MX_SPI2_Init();

  /* Disable JTAG to release PA15 (JTMS) as GPIO */
  __HAL_RCC_AFIO_CLK_ENABLE();
  __HAL_AFIO_REMAP_SWJ_NOJTAG();

  /* USER CODE BEGIN 2 */
  
  /* 初始化LED控制 */
  LED_Init();
  
  /* 初始化风扇控制（包含FG捕获初始化） */
  fanctrl_init();
  
  /* 短暂延时 */
  HAL_Delay(1000);
  
  /* 启动SPI2从机通信 */
  SPI2_SlaveComm_Start();
  
  /* 注册命令处理函数 */
  SPI2_RegisterCommandHandler(Main_SPI_CommandHandler);
  USART1_RegisterCommandHandler(Main_USART1_CommandHandler);
  
  /* 启动UART中断接收 */
  extern uint8_t rx_buffer[];
  extern volatile uint8_t rx_index;
  rx_index = 0;
  HAL_UART_Receive_IT(&huart1, &rx_buffer[rx_index], 1);
  
  /* 启动USART2中断接收 */
  extern uint8_t rx_buffer_usart2[];
  extern volatile uint8_t rx_index_usart2;
  rx_index_usart2 = 0;
  HAL_UART_Receive_IT(&huart2, &rx_buffer_usart2[rx_index_usart2], 1);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    
    /* 轮询SPI NSS状态 */
    SPI2_SlaveComm_Poll();
    
    /* 处理SPI命令 */
    SPI2_Process();
    
    /* 处理USART1命令 */
    USART1_Process();
    
    /* 处理USART2命令 */
    USART2_Process();
    
    /* LED控制（1秒周期） */
    LED_Process();
    
    /* FG捕获处理 */
    FG_Process();
    
    /* NTC温度读取（1秒周期，仅在使能时） */
    if (ntc_stream_enable && (HAL_GetTick() - last_ntc_tick >= 1000U))
    {
        last_ntc_tick += 1000U;
        /* NTC_Process() 可以在这里调用 */
    }
    
    /* FG频率打印（1秒周期，仅在使能时） */
    if (fg_frq_enable && (HAL_GetTick() - last_fg_tick >= 1000U))
    {
        last_fg_tick += 1000U;
        FG_PrintStatus();
    }

    HAL_Delay(1);
  }
  /* USER CODE END 3 */
}

/**
  * @brief SPI命令处理函数
  */
static void Main_SPI_CommandHandler(uint8_t *cmd, uint8_t cmd_len, 
                                     uint8_t *response, uint8_t *response_len, 
                                     uint16_t response_maxlen)
{
    (void)cmd_len;
    
    trim_cmd((char *)cmd);
    uint8_t en0 = 0, en1 = 0, a0 = 0, a1 = 0, a2 = 0;
    uint8_t pwm_id = 0, off_percent = 0;
    
    /* NTC_START 指令 */
    if (equals_ignore_case((char *)cmd, "NTC_START")) {
        ntc_stream_enable = 1;
        last_ntc_tick = HAL_GetTick();
        strncpy((char *)response, "[OK] NTC stream ON", response_maxlen - 1);
        *response_len = strlen((char *)response);
    }
    /* NTC_STOP 指令 */
    else if (equals_ignore_case((char *)cmd, "NTC_STOP")) {
        ntc_stream_enable = 0;
        strncpy((char *)response, "[OK] NTC stream OFF", response_maxlen - 1);
        *response_len = strlen((char *)response);
    }
    /* FG_FRQ_START 指令 */
    else if (equals_ignore_case((char *)cmd, "FG_FRQ_START")) {
        fg_frq_enable = 1;
        last_fg_tick = HAL_GetTick();
        strncpy((char *)response, "[OK] FG frequency read started", response_maxlen - 1);
        *response_len = strlen((char *)response);
    }
    /* FG_FRQ_STOP 指令 */
    else if (equals_ignore_case((char *)cmd, "FG_FRQ_STOP")) {
        fg_frq_enable = 0;
        float freq = FG_GetFrequency();
        float rpm = FG_CalculateRPM();
        snprintf((char *)response, response_maxlen, 
                 "[FG] Freq: %.2f Hz, RPM: %.0f", freq, rpm);
        *response_len = strlen((char *)response);
    }
    /* FGRDxxxx 指令 */
    else if (parse_fgrd_cmd((const char *)cmd, &en0, &en1, &a0, &a1, &a2)) {
        FG_StartOneShot(en0, en1, a0, a1, a2);
        
        /* 等待捕获完成（简化版，实际应用中可能需要异步处理） */
        uint32_t start = HAL_GetTick();
        while (FG_GetOneShotResult(NULL, NULL, NULL) == 0 && 
               (HAL_GetTick() - start) < 2100)
        {
            FG_Process();
            HAL_Delay(1);
        }
        
        float freq, rpm;
        uint32_t captures;
        FG_GetOneShotResult(&freq, &rpm, &captures);
        
        snprintf((char *)response, response_maxlen,
                 "[FGRD] Freq: %.2f Hz, RPM: %.0f, Captures: %lu",
                 freq, rpm, captures);
        *response_len = strlen((char *)response);
    }
    /* xPWMxxx 指令 */
    else if (parse_xpwm_cmd((const char *)cmd, &pwm_id, &off_percent)) {
        uint8_t duty_on_percent = (uint8_t)(100U - off_percent);
        fan_pwm_set_single(duty_on_percent, pwm_id);
        snprintf((char *)response, response_maxlen,
                 "[OK] PWM%u duty=%u%%", pwm_id, duty_on_percent);
        *response_len = strlen((char *)response);
    }
    /* 其他指令：转发到USART2 (TEC) */
    else {
        HAL_UART_Transmit(&huart2, cmd, strlen((char *)cmd), 100);
        uint8_t crlf[] = "\r\n";
        HAL_UART_Transmit(&huart2, crlf, 2, 100);
        
        /* 等待TEC响应 */
        uint32_t wait_start = HAL_GetTick();
        uint8_t tec_response[128];
        uint8_t tec_resp_len = 0;
        while ((HAL_GetTick() - wait_start) < 500 && tec_resp_len < sizeof(tec_response) - 1) {
            uint8_t rx_data[32];
            uint8_t len = USART2_GetReceivedData(rx_data, sizeof(rx_data));
            if (len > 0) {
                if (tec_resp_len + len < sizeof(tec_response)) {
                    memcpy(&tec_response[tec_resp_len], rx_data, len);
                    tec_resp_len += len;
                }
                wait_start = HAL_GetTick();
            }
            HAL_Delay(1);
        }
        
        if (tec_resp_len > 0) {
            tec_response[tec_resp_len] = '\0';
            strncpy((char *)response, (char *)tec_response, response_maxlen - 1);
            response[response_maxlen - 1] = '\0';
            *response_len = strlen((char *)response);
        } else {
            strncpy((char *)response, "[ERR] TEC timeout", response_maxlen - 1);
            *response_len = strlen((char *)response);
        }
    }
}

/**
  * @brief USART1命令处理函数
  */
static void Main_USART1_CommandHandler(uint8_t *cmd, uint8_t cmd_len)
{
    (void)cmd_len;
    
    trim_cmd((char *)cmd);
    uint8_t en0 = 0, en1 = 0, a0 = 0, a1 = 0, a2 = 0;
    uint8_t pwm_id = 0, off_percent = 0;

    /* NTC_START 指令 */
    if (equals_ignore_case((char *)cmd, "NTC_START")) {
        ntc_stream_enable = 1;
        last_ntc_tick = HAL_GetTick();
        printf("[OK] NTC stream ON\r\n");
    }
    /* NTC_STOP 指令 */
    else if (equals_ignore_case((char *)cmd, "NTC_STOP")) {
        ntc_stream_enable = 0;
        printf("[OK] NTC stream OFF\r\n");
    }
    /* FGDEBUG 指令 */
    else if (equals_ignore_case((char *)cmd, "FGDEBUG")) {
        FG_SetDebugEnable(1);
        printf("[OK] FG debug ON\r\n");
    }
    /* FGDEBUGOFF 指令 */
    else if (equals_ignore_case((char *)cmd, "FGDEBUGOFF")) {
        FG_SetDebugEnable(0);
        printf("[OK] FG debug OFF\r\n");
    }
    /* FGRESET 指令 */
    else if (equals_ignore_case((char *)cmd, "FGRESET")) {
        FG_ResetCapture();
        printf("[OK] FG capture reset\r\n");
    }
    /* FG_FRQ_START 指令 */
    else if (equals_ignore_case((char *)cmd, "FG_FRQ_START")) {
        fg_frq_enable = 1;
        last_fg_tick = HAL_GetTick();
        printf("[OK] FG frequency read started\r\n");
    }
    /* FG_FRQ_STOP 指令 */
    else if (equals_ignore_case((char *)cmd, "FG_FRQ_STOP")) {
        fg_frq_enable = 0;
        FG_PrintStatus();
        printf("[OK] FG frequency read stopped\r\n");
    }
    /* FGRDxxxx 指令 */
    else if (parse_fgrd_cmd((const char *)cmd, &en0, &en1, &a0, &a1, &a2)) {
        FG_StartOneShot(en0, en1, a0, a1, a2);
        printf("[OK] FGRD pending capture\r\n");
    }
    /* xPWMxxx 指令 */
    else if (parse_xpwm_cmd((const char *)cmd, &pwm_id, &off_percent)) {
        uint8_t duty_on_percent = (uint8_t)(100U - off_percent);
        fan_pwm_set_single(duty_on_percent, pwm_id);
        printf("[OK] PWM%u duty=%u%%\r\n", pwm_id, duty_on_percent);
    }
    /* TEC Controller Command: Forward to USART2 */
    else {
        HAL_UART_Transmit(&huart2, cmd, strlen((char *)cmd), 100);
        uint8_t crlf[] = "\r\n";
        HAL_UART_Transmit(&huart2, crlf, 2, 100);
    }
}

/**
  * @brief USART2命令处理函数
  */
static void Main_USART2_CommandHandler(uint8_t *cmd, uint8_t cmd_len)
{
    (void)cmd_len;
    /* 将TEC响应转发到USART1 */
    printf("[TEC] %s\r\n", cmd);
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.HSEState = RCC_HSE_OFF;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL16;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

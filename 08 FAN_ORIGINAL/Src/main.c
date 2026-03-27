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
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

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


/* 设置单路 PWM 占空比（PUMP/FAN1/2/3 共用 TIM3_CH1~CH4）
 * pwm_id: 0->PUMP_PWM, 1->FAN_PWM1, 2->FAN_PWM2, 3->FAN_PWM3
 * duty_percent: 期望占空比(0~100)，100 表示满转
 */
static void PWM_SetDutyPercent_ById(uint8_t pwm_id, uint8_t duty_percent)
{
    uint32_t tim_channel = 0;
    if (pwm_id == 0) tim_channel = TIM_CHANNEL_1;
    else if (pwm_id == 1) tim_channel = TIM_CHANNEL_2;
    else if (pwm_id == 2) tim_channel = TIM_CHANNEL_3;
    else if (pwm_id == 3) tim_channel = TIM_CHANNEL_4;
    else return;

    uint32_t arr = __HAL_TIM_GET_AUTORELOAD(&htim3);
    uint32_t compare = ((arr + 1U) * duty_percent) / 100U;
    if (compare > arr) {
        compare = arr;
    }

    __HAL_TIM_SET_COMPARE(&htim3, tim_channel, compare);
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


/* 解析 xPWMxxx：
 * xPWMxxx:
 *   x: 0~3, 对应 PUMP_PWM/FAN_PWM1/FAN_PWM2/FAN_PWM3
 *   xxx: 关断百分比(off%)，占空比 = 100 - off%
 *       例如 040 => off=40% => duty=60%
 *             080 => off=80% => duty=20%
 */
static uint8_t parse_xpwm_cmd(const char *cmd, uint8_t *out_pwm_id, uint8_t *out_off_percent)
{
    if (cmd == NULL || out_pwm_id == NULL || out_off_percent == NULL) return 0;
    if (strlen(cmd) != 7) return 0; /* "xPWM" + 3 digits */

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

/* 解析 FGRDxxxx：
 * xxxx[0]：EN选择位（0 -> EN0=0, EN1=1；1 -> EN0=1, EN1=0），EN0和EN1互斥
 * xxxx[1]：MUX_ADDR0
 * xxxx[2]：MUX_ADDR1
 * xxxx[3]：MUX_ADDR2
 * 例如 FGRD0100：EN0=0, EN1=1, ADDR0=0, ADDR1=0, ADDR2=0
 *      FGRD1001：EN0=1, EN1=0, ADDR0=0, ADDR1=0, ADDR2=1
 */
static uint8_t parse_fgrd_cmd(const char *cmd,
                              uint8_t *out_en0, uint8_t *out_en1,
                              uint8_t *out_addr0, uint8_t *out_addr1, uint8_t *out_addr2)
{
    if (cmd == NULL ||
        out_en0 == NULL || out_en1 == NULL ||
        out_addr0 == NULL || out_addr1 == NULL || out_addr2 == NULL) {
        return 0;
    }

    if (strlen(cmd) != 8) return 0; /* "FGRD" + 4 digits */

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

    /* 仅允许 0/1 */
    if (cmd[4] != '0' && cmd[4] != '1') return 0;
    if (cmd[5] != '0' && cmd[5] != '1') return 0;
    if (cmd[6] != '0' && cmd[6] != '1') return 0;
    if (cmd[7] != '0' && cmd[7] != '1') return 0;

    /* EN0和EN1互斥：第1位为0时EN0=0,EN1=1；第1位为1时EN0=1,EN1=0 */
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

// FG信号捕获相关变量
static volatile uint32_t fg_last_time = 0;
static volatile uint32_t fg_current_time = 0;
static volatile uint32_t fg_period = 0;
static volatile float fg_frequency = 0.0f;
static volatile uint32_t last_fg_update_time = 0;
static volatile uint32_t fg_capture_count = 0;  // 捕获计数
static volatile uint8_t fg_debug_enabled = 0;   // debug开关，默认关闭

/* FGRDxxxx 一键回读一次 */
static uint8_t fg_one_shot_active = 0;
static uint32_t fg_one_shot_start_tick = 0;
static uint32_t fg_one_shot_settle_until = 0;
static uint32_t fg_one_shot_timeout_ms = 2000U;
static uint8_t fg_one_shot_en0 = 0;
static uint8_t fg_one_shot_en1 = 0;
static uint8_t fg_one_shot_addr0 = 0;
static uint8_t fg_one_shot_addr1 = 0;
static uint8_t fg_one_shot_addr2 = 0;

/* 设置 CD4051/模拟开关的 MUX 引脚电平（按电平值写入） */
static void MUX_SetFGMuxPins(uint8_t en0, uint8_t en1,
                              uint8_t addr0, uint8_t addr1, uint8_t addr2)
{
    HAL_GPIO_WritePin(MUX_EN0_GPIO_Port, MUX_EN0_Pin, en0 ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MUX_EN1_GPIO_Port, MUX_EN1_Pin, en1 ? GPIO_PIN_SET : GPIO_PIN_RESET);

    HAL_GPIO_WritePin(MUX_ADDR0_GPIO_Port, MUX_ADDR0_Pin, addr0 ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MUX_ADDR1_GPIO_Port, MUX_ADDR1_Pin, addr1 ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MUX_ADDR2_GPIO_Port, MUX_ADDR2_Pin, addr2 ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

// 配置CD4051地址，选择Y4通道（FAN11）
static void MUX_SelectY4(void)
{
    // 使能MUX_EN1（低电平使能）
    HAL_GPIO_WritePin(MUX_EN1_GPIO_Port, MUX_EN1_Pin, GPIO_PIN_RESET);
    
    // 配置地址：Y4通道对应CBA=100
    // 根据原理图，MUX_ADDR与CD4051B的A0-A2连接顺序是反的
    // MUX_ADDR2 → A0
    // MUX_ADDR1 → A1
    // MUX_ADDR0 → A2
    
    // C (A2) = 1 → MUX_ADDR0 = 1
    HAL_GPIO_WritePin(MUX_ADDR0_GPIO_Port, MUX_ADDR0_Pin, GPIO_PIN_RESET);
    // B (A1) = 0 → MUX_ADDR1 = 0
    HAL_GPIO_WritePin(MUX_ADDR1_GPIO_Port, MUX_ADDR1_Pin, GPIO_PIN_RESET);
    // A (A0) = 0 → MUX_ADDR2 = 0
    HAL_GPIO_WritePin(MUX_ADDR2_GPIO_Port, MUX_ADDR2_Pin, GPIO_PIN_SET);
}

// TIM2输入捕获回调函数 - 使用CH2(PA1)捕获FG信号
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2 && htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)
    {
        static uint32_t callback_count = 0;
        callback_count++;
			
        uint32_t captured_value = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2);

        // 打印回调信息（前几次）
        if (fg_debug_enabled && callback_count <= 5)
        {
            UART_Printf("[FG DEBUG] Callback #%lu: TIM2_CH2, Value=%lu\r\n",
                        callback_count, captured_value);
        }

        // 第一次捕获，只记录时间
        if (fg_capture_count == 0)
        {
            fg_last_time = captured_value;
            fg_current_time = captured_value;
            fg_capture_count++;
            last_fg_update_time = HAL_GetTick();

            if (fg_debug_enabled)
            {
                UART_Printf("[FG DEBUG] First capture: %lu\r\n", captured_value);
            }
            return;
        }

        fg_last_time = fg_current_time;
        fg_current_time = captured_value;

        if (fg_current_time > fg_last_time)
        {
            fg_period = fg_current_time - fg_last_time;
        }
        else
        {
            // 计数器溢出处理
            fg_period = (0xFFFF - fg_last_time) + fg_current_time + 1;
        }

        // 计算频率：定时器时钟10kHz / 周期
        if (fg_period > 0)
        {
            fg_frequency = 10000.0f / fg_period;
        }
        else
        {
            fg_frequency = 0.0f;
        }

        fg_capture_count++;

        // Debug打印（前10次捕获或每100次捕获一次）
        if (fg_debug_enabled && (fg_capture_count <= 10 || fg_capture_count % 100 == 0))
        {
            UART_Printf("[FG DEBUG] Capture #%lu: curr=%lu, last=%lu, period=%lu, freq=%.2f Hz\r\n",
                        fg_capture_count, fg_current_time, fg_last_time, fg_period, fg_frequency);
        }

        // 更新最后捕获时间
        last_fg_update_time = HAL_GetTick();
    }
}

/**
 * @brief  发送字符串到串口
 * @param  msg: 要发送的字符串
 * @retval void
 */
void UART_SendString(const char *msg)
{
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
}

/**
 * @brief  发送格式化字符串到串口
 * @param  format: 格式化字符串
 * @param  ...: 可变参数
 * @retval void
 */
void UART_Printf(const char *format, ...)
{
    char buffer[128];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    UART_SendString(buffer);
}

/**
 * @brief  扫描I2C总线上的设备
 * @retval void
 */
void I2C_Scan(void)
{
    // 专门测试TLA2528地址
    UART_Printf("TLA2528 7-bit: 0x%02X, Write: 0x%02X, Read: 0x%02X\r\n", 
                TLA2528_I2C_ADDR, TLA2528_I2C_WRITE_ADDR, TLA2528_I2C_READ_ADDR);
    
    // 测试TLA2528写地址
    HAL_StatusTypeDef status = HAL_I2C_IsDeviceReady(&hi2c, TLA2528_I2C_WRITE_ADDR, 3, 100);
    if (status == HAL_OK)
    {
        UART_SendString("[PASS] TLA2528 device detected at expected address\r\n");
    }
    else
    {
        UART_SendString("[FAIL] TLA2528 device not detected at expected address\r\n");
    }
}

/**
 * @brief  TLA2528测试序列
 * @retval void
 */
void tla2528_test_sequence(void)
{
    UART_SendString("\r\n=== TLA2528 Test Sequence ===\r\n");
    
    // 测试I2C通信
    // 扫描I2C总线
    I2C_Scan();
    
    // 尝试检测TLA2528设备
    UART_SendString("\r\nDetecting TLA2528 device...\r\n");
    UART_Printf("Using I2C address: 0x%02X (7-bit), 0x%02X (write), 0x%02X (read)\r\n", 
                TLA2528_I2C_ADDR, TLA2528_I2C_WRITE_ADDR, TLA2528_I2C_READ_ADDR);
    
    // 方法1：使用HAL_I2C_IsDeviceReady检测设备
    UART_SendString("\r\nMethod 1: Using HAL_I2C_IsDeviceReady...\r\n");
    
    // 检查I2C状态
    HAL_I2C_StateTypeDef state = HAL_I2C_GetState(&hi2c);
    char debug_buf[64];
    sprintf(debug_buf, "[DEBUG] I2C State before IsDeviceReady: %d\r\n", state);
    UART_SendString(debug_buf);
    
    HAL_StatusTypeDef status = HAL_I2C_IsDeviceReady(&hi2c, TLA2528_I2C_WRITE_ADDR, 3, 100);
    sprintf(debug_buf, "[DEBUG] IsDeviceReady status: %d\r\n", status);
    UART_SendString(debug_buf);
    
    if (status == HAL_OK)
    {
        UART_SendString("[PASS] TLA2528 device detected (IsDeviceReady)\r\n");
    }
    else
    {
        UART_SendString("[FAIL] TLA2528 device not detected (IsDeviceReady)\r\n");
    }
    
    // 方法2：直接尝试读取系统状态寄存器
    UART_SendString("\r\nMethod 2: Directly reading system registers...\r\n");
    uint8_t system_status = 0;
    HAL_StatusTypeDef read_status = TLA2528_ReadReg(TLA2528_REG_SYSTEM_STATUS, &system_status);
    
    if (read_status == HAL_OK)
    {
        UART_SendString("[PASS] TLA2528 device detected\r\n");
        UART_Printf("[PASS] Read SYSTEM_STATUS Reg: 0x%02X (Expected reset: 0x80)\r\n", system_status);
        
        // 读取通用配置寄存器
        uint8_t general_cfg = 0;
        if (TLA2528_ReadReg(TLA2528_REG_GENERAL_CFG, &general_cfg) == HAL_OK)
        {
            UART_Printf("[PASS] Read GENERAL_CFG Reg: 0x%02X (Expected reset: 0x00)\r\n", general_cfg);
        }
        else
        {
            UART_SendString("[FAIL] Read GENERAL_CFG Reg Error\r\n");
        }
        
        // 读取通道选择寄存器
        uint8_t channel_sel = 0;
        if (TLA2528_ReadReg(TLA2528_REG_CHANNEL_SEL, &channel_sel) == HAL_OK)
        {
            UART_Printf("[PASS] Read CHANNEL_SEL Reg: 0x%02X (Expected reset: 0x00)\r\n", channel_sel);
        }
        else
        {
            UART_SendString("[FAIL] Read CHANNEL_SEL Reg Error\r\n");
        }
        
        // 读取其他寄存器以验证
        uint8_t data_cfg = 0;
        if (TLA2528_ReadReg(TLA2528_REG_DATA_CFG, &data_cfg) == HAL_OK)
        {
            UART_Printf("[PASS] Read DATA_CFG Reg: 0x%02X (Expected reset: 0x00)\r\n", data_cfg);
        }
        else
        {
            UART_SendString("[FAIL] Read DATA_CFG Reg Error\r\n");
        }
        
        uint8_t opmode_cfg = 0;
        if (TLA2528_ReadReg(TLA2528_REG_OPMODE_CFG, &opmode_cfg) == HAL_OK)
        {
            UART_Printf("[PASS] Read OPMODE_CFG Reg: 0x%02X (Expected reset: 0x00)\r\n", opmode_cfg);
        }
        else
        {
            UART_SendString("[FAIL] Read OPMODE_CFG Reg Error\r\n");
        }
        
        // 测试寄存器写入和读取功能
        UART_SendString("\r\nTesting register write/read functionality...\r\n");
        
        // 测试CHANNEL_SEL寄存器（可修改）
        uint8_t ch_sel_default = 0;
        if (TLA2528_ReadReg(TLA2528_REG_CHANNEL_SEL, &ch_sel_default) == HAL_OK)
        {
            UART_Printf("[PASS] CHANNEL_SEL default value: 0x%02X\r\n", ch_sel_default);
        }
        else
        {
            UART_SendString("[FAIL] Read CHANNEL_SEL default value error\r\n");
        }
        
        // 修改CHANNEL_SEL寄存器为通道3
        uint8_t ch_sel_new = 0x04;
        if (TLA2528_WriteReg(TLA2528_REG_CHANNEL_SEL, ch_sel_new) == HAL_OK)
        {
            UART_Printf("[PASS] Write CHANNEL_SEL to: 0x%02X\r\n", ch_sel_new);
        }
        else
        {
            UART_SendString("[FAIL] Write CHANNEL_SEL error\r\n");
        }
        
        // 读取修改后的CHANNEL_SEL值
        uint8_t ch_sel_readback = 0;
        if (TLA2528_ReadReg(TLA2528_REG_CHANNEL_SEL, &ch_sel_readback) == HAL_OK)
        {
            UART_Printf("[PASS] Read back CHANNEL_SEL: 0x%02X\r\n", ch_sel_readback);
            if (ch_sel_readback == ch_sel_new)
            {
                UART_SendString("[PASS] Register write/read test PASSED\r\n");
            }
            else
            {
                UART_SendString("[FAIL] Register write/read test FAILED\r\n");
            }
        }
        else
        {
            UART_SendString("[FAIL] Read back CHANNEL_SEL error\r\n");
        }
        
        // 恢复CHANNEL_SEL为默认值
        if (TLA2528_WriteReg(TLA2528_REG_CHANNEL_SEL, ch_sel_default) == HAL_OK)
        {
            UART_Printf("[PASS] Restore CHANNEL_SEL to default: 0x%02X\r\n", ch_sel_default);
        }
        else
        {
            UART_SendString("[FAIL] Restore CHANNEL_SEL error\r\n");
        }
        
        // 触发转换并读取通道0
        UART_SendString("\r\nTesting ADC conversion...\r\n");
        if (TLA2528_TriggerConversion() == HAL_OK)
        {
            UART_SendString("[PASS] Trigger conversion success\r\n");
            HAL_Delay(10);
            
            uint16_t ch0_value = 0;
            if (TLA2528_ReadChannel(0, &ch0_value, NULL) == HAL_OK)
            {
                UART_Printf("[PASS] Read CH0 NTC Raw Value: %u\r\n", ch0_value);
                
                if (ch0_value == 0 || ch0_value == 4095)
                {
                    UART_SendString("[WARNING] NTC sensor may be open or shorted\r\n");
                }
                else
                {
                    UART_SendString("[INFO] NTC sensor reading is normal\r\n");
                }
            }
            else
            {
                UART_SendString("[FAIL] Read Channel 0 Error\r\n");
            }
        }
        else
        {
            UART_SendString("[FAIL] Trigger conversion error\r\n");
        }
    }
    else
    {
        UART_SendString("[FAIL] Failed to read CONFIG register\r\n");
        UART_Printf("HAL status: %d\r\n", status);
        
        // 尝试方法3：使用HAL_I2C_Master_Transmit直接发送数据
        UART_SendString("\r\nMethod 3: Trying direct I2C transmission...\r\n");
        uint8_t test_data = 0x00;
        status = HAL_I2C_Master_Transmit(&hi2c, TLA2528_I2C_WRITE_ADDR, &test_data, 1, 100);
        if (status == HAL_OK)
        {
            UART_SendString("[PASS] Direct I2C transmission successful\r\n");
        }
        else
        {
            UART_SendString("[FAIL] Direct I2C transmission failed\r\n");
            UART_Printf("HAL status: %d\r\n", status);
        }
    }
    
    UART_SendString("\r\n=== Test Sequence Complete ===\r\n");
}

// NTC相关定义
#define NTC_CHANNEL 4           // CH21_NTC连接到通道4
#define NTC_R0 10000.0f         // NTC在25℃时的电阻值
#define NTC_B 3975.0f            // NTC的B值
#define NTC_RREF 4700.0f         // 串联电阻值
#define NTC_VCC 3.3f             // 电源电压
#define NTC_T0 298.15f           // 参考温度（25℃）
#define NTC_ADC_MAX 4095.0f      // 12位ADC最大值

/**
 * @brief  计算NTC温度
 * @param  adc_value: ADC转换值
 * @retval float: 温度值（摄氏度）
 */
float NTC_CalculateTemperature(uint16_t adc_value)
{
    // 检查ADC值是否有效
    if (adc_value == 0) {
        return -273.15f; // 无效值
    }
    
    if (adc_value >= 4095) {
        return 150.0f; // 上限温度
    }
    
    // 计算电压
    float voltage = (float)adc_value / NTC_ADC_MAX * NTC_VCC;
    
    // 防止除零错误
    if (voltage >= NTC_VCC - 0.001f) {
        return 150.0f; // 上限温度
    }
    
    // 计算NTC电阻
    float rntc = (NTC_RREF * voltage) / (NTC_VCC - voltage);
    
    // 使用B值公式计算温度
    float inv_t = 1.0f / NTC_T0 + (1.0f / NTC_B) * logf(rntc / NTC_R0);
    float temp_k = 1.0f / inv_t;
    float temp_c = temp_k - 273.15f;
    
    // 温度范围限制
    if (temp_c < -40.0f) {
        return -40.0f;
    }
    if (temp_c > 125.0f) {
        return 125.0f;
    }
    
    return temp_c;
}

/**
 * @brief  读取NTC温度
 * @param  temperature: 温度值存储地址
 * @retval HAL_StatusTypeDef: 操作状态
 */
HAL_StatusTypeDef NTC_ReadTemperature(float *temperature)
{
    uint16_t adc_value = 0;

    // 读取通道4的ADC值
    HAL_StatusTypeDef status = TLA2528_ReadChannel(NTC_CHANNEL, &adc_value, NULL);
    if (status != HAL_OK)
    {
        return status;
    }

    // 计算温度
    *temperature = NTC_CalculateTemperature(adc_value);

    return HAL_OK;
}

/**
 * @brief  扫描所有寄存器
 * @retval void
 */
void ScanAllRegisters(void)
{
    UART_SendString("\r\n=== Scanning All Registers (0x00-0x1F) ===\r\n");

    for (uint8_t addr = 0x00; addr <= 0x1F; addr++)
    {
        uint8_t value = 0xFF;
        HAL_StatusTypeDef status = TLA2528_ReadReg(addr, &value);

        if (status == HAL_OK)
        {
            UART_Printf("Reg[0x%02X] = 0x%02X (%3d)", addr, value, value);

            // 每4个寄存器换行
            if ((addr + 1) % 4 == 0)
            {
                UART_SendString("\r\n");
            }
            else
            {
                UART_SendString("  ");
            }
        }
        else
        {
            UART_Printf("Reg[0x%02X] = ERROR  ", addr);
        }
    }
    UART_SendString("\r\n=== Scan Complete ===\r\n");
}

/**
 * @brief  I2C通信调试函数
 * @retval void
 */
void I2C_DebugTest(void)
{
    UART_SendString("\r\n=== I2C Communication Debug ===\r\n");

    // 步骤1：检查I2C设备是否响应
    UART_SendString("[Step 1] Checking if TLA2528 is responding...\r\n");
    HAL_StatusTypeDef status = HAL_I2C_IsDeviceReady(&hi2c, TLA2528_I2C_WRITE_ADDR, 3, 100);
    if (status == HAL_OK)
    {
        UART_SendString("[PASS] TLA2528 device is responding\r\n");
    }
    else
    {
        UART_SendString("[FAIL] TLA2528 device is NOT responding\r\n");
        return;
    }

    // 步骤2：读取系统状态寄存器
    UART_SendString("\r\n[Step 2] Reading SYSTEM_STATUS register (0x00)...\r\n");
    uint8_t system_status = 0;
    status = TLA2528_ReadReg(TLA2528_REG_SYSTEM_STATUS, &system_status);
    if (status == HAL_OK)
    {
        UART_Printf("[PASS] SYSTEM_STATUS = 0x%02X\r\n", system_status);
    }
    else
    {
        UART_SendString("[FAIL] Failed to read SYSTEM_STATUS\r\n");
        return;
    }

    // 步骤3：扫描所有寄存器
    ScanAllRegisters();

    // 步骤4：配置引脚为ADC模式
    UART_SendString("\r\n[Step 3] Configuring all pins to ADC mode...\r\n");
    // PIN_CFG格式：每个引脚2位，00=ADC输入
    // 低字节控制AIN0-3，高字节控制AIN4-7
    TLA2528_WriteReg(TLA2528_REG_PIN_CFG, 0x00);  // AIN0-3 = ADC
    TLA2528_WriteReg(TLA2528_REG_PIN_CFG_HIGH, 0x00);  // AIN4-7 = ADC

    // 验证设置
    uint8_t pin_cfg_low = 0, pin_cfg_high = 0;
    TLA2528_ReadReg(TLA2528_REG_PIN_CFG, &pin_cfg_low);
    TLA2528_ReadReg(TLA2528_REG_PIN_CFG_HIGH, &pin_cfg_high);
    UART_Printf("[PASS] PIN_CFG LOW = 0x%02X, HIGH = 0x%02X\r\n", pin_cfg_low, pin_cfg_high);

    // 步骤4：仅读取当前通道选择寄存器（不再强制改成 4）
    UART_SendString("\r\n[Step 4] Reading CHANNEL_SEL register...\r\n");
    uint8_t channel_sel = 0;
    status = TLA2528_ReadReg(TLA2528_REG_CHANNEL_SEL, &channel_sel);
    if (status == HAL_OK)
    {
        UART_Printf("[PASS] CHANNEL_SEL = 0x%02X\r\n", channel_sel);
    }
    else
    {
        UART_SendString("[FAIL] Failed to read CHANNEL_SEL\r\n");
        return;
    }

    // 步骤5：通过 CNVST 位触发一次转换（走规范的 opcode 流程）
    UART_SendString("\r\n[Step 5] Triggering ADC conversion via CNVST...\r\n");
    status = TLA2528_TriggerConversion();
    if (status == HAL_OK)
    {
        UART_SendString("[PASS] Conversion triggered\r\n");
    }
    else
    {
        UART_SendString("[FAIL] Failed to trigger conversion\r\n");
        return;
    }

    HAL_Delay(10); // 转换时间在 us 级，这里预留 10ms 保险

    // 步骤6：直接读取一次 ADC 原始数据（使用数据帧方式）
    UART_SendString("\r\n[Step 6] Reading ADC data frame...\r\n");
    uint16_t adc_value = 0;
    uint8_t raw[2] = {0};
    status = TLA2528_ReadADC(&adc_value, raw);
    if (status == HAL_OK)
    {
        UART_Printf("[PASS] Raw bytes: [0x%02X, 0x%02X]\r\n", raw[0], raw[1]);
        UART_Printf("[PASS] ADC value: %u\r\n", adc_value);
    }
    else
    {
        UART_SendString("[FAIL] Failed to read ADC data frame\r\n");
    }

    UART_SendString("\r\n=== Debug Test Complete ===\r\n");
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
  MX_TIM4_Init(); // 初始化TIM4用于LED PWM
  MX_TIM3_Init(); // 初始化TIM3用于FAN PWM
  MX_TIM2_Init(); // 初始化TIM2用于FG信号捕获
  MX_I2C_Init(); // 初始化I2C用于TLA2528通信
  MX_SPI2_Init(); // SPI2 从机：PB12~PB15
  // MX_USART3_UART_Init(); // 不需要使用USART3


  /* Disable JTAG to release PA15 (JTMS) as GPIO */
  __HAL_RCC_AFIO_CLK_ENABLE();
  __HAL_AFIO_REMAP_SWJ_NOJTAG();

  /* 启用窗口看门狗 */
  // MX_WWDG_Init(); // 暂时禁用WWDG，排查重启问题

  /* 启动TIM4 PWM通道 */
  HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_3);
  HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_4);

  /* 启动 TIM3 PWM（PC6~PC9：PUMP/FAN1/FAN2/FAN3，TIM3启用FULL REMAP） */
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_4);
  /* 上电/复位后：所有风扇/水泵 PWM 置为 0%（低电平） */
  PWM_SetDutyPercent_ById(0, 0U);
  PWM_SetDutyPercent_ById(1, 0U);
  PWM_SetDutyPercent_ById(2, 0U);
  PWM_SetDutyPercent_ById(3, 0U);





  /* 启动TIM2输入捕获中断（PA1 - TIM2_CH2） */
  if (HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_2) != HAL_OK)
  {
    UART_SendString("[ERROR] Failed to start TIM2 IC interrupt\r\n");
  }
  else
  {
    UART_SendString("[INFO] TIM2 CH2 (PA1) input capture started\r\n");
  }

  /* 初始化FG信号更新时间 */
  last_fg_update_time = HAL_GetTick();

  /* 打印 TIM3 实际配置，便于示波器对照 */
  UART_Printf("[INFO] TIM3 FULL REMAP enabled: PC6=CH1, PC7=CH2, PC8=CH3, PC9=CH4\r\n");
  UART_Printf("[INFO] TIM3 PSC=%lu ARR=%lu CCR1=%lu CCR2=%lu CCR3=%lu CCR4=%lu\r\n",
              (uint32_t)htim3.Instance->PSC,
              (uint32_t)__HAL_TIM_GET_AUTORELOAD(&htim3),
              (uint32_t)__HAL_TIM_GET_COMPARE(&htim3, TIM_CHANNEL_1),
              (uint32_t)__HAL_TIM_GET_COMPARE(&htim3, TIM_CHANNEL_2),
              (uint32_t)__HAL_TIM_GET_COMPARE(&htim3, TIM_CHANNEL_3),
              (uint32_t)__HAL_TIM_GET_COMPARE(&htim3, TIM_CHANNEL_4));
  
  /* 初始化LED状态 - 通过PWM控制 */
  __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_3, 500000); // 50%占空比
  __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_4, 0); // 0%占空比

  /* USER CODE BEGIN 2 */
  /* 短暂延时，确保串口助手准备就绪 */
  HAL_Delay(1000);
  
  /* 启动SPI2从机通信 */
  SPI2_SlaveComm_Start();
  
  /* 执行TLA2528测试序列 */
  tla2528_test_sequence();

  /* 配置TLA2528的操作模式（参考数据手册寄存器表） */
  UART_SendString("\r\nConfiguring TLA2528 for NTC reading...\r\n");

  // 1) 确保所有通道为模拟输入（GENERAL_CFG.CH_RST = 1）
  //    写 0x04：CH_RST = 1，其他位为 0
  if (TLA2528_WriteReg(TLA2528_REG_GENERAL_CFG, 0x04) == HAL_OK)
  {
      UART_SendString("[PASS] All channels forced to analog inputs\r\n");
  }
  else
  {
      UART_SendString("[FAIL] Failed to set CH_RST bit in GENERAL_CFG\r\n");
  }

  // 2) 数据配置：不附加 Channel ID，不启用固定模式调试（保持 0x00 即可）
  if (TLA2528_WriteReg(TLA2528_REG_DATA_CFG, 0x00) == HAL_OK)
  {
      UART_SendString("[PASS] DATA_CFG configured\r\n");
  }
  else
  {
      UART_SendString("[FAIL] Failed to configure DATA_CFG\r\n");
  }

  // 3) 过采样配置：不做过采样，OSR=0（单次 12bit 转换）
  if (TLA2528_WriteReg(TLA2528_REG_OSR_CFG, 0x00) == HAL_OK)
  {
      UART_SendString("[PASS] OSR_CFG configured\r\n");
  }
  else
  {
      UART_SendString("[FAIL] Failed to configure OSR_CFG\r\n");
  }

  // 4) 操作模式：使用高速内部振荡器，最大采样速率，CLK_DIV=0
  if (TLA2528_WriteReg(TLA2528_REG_OPMODE_CFG, 0x00) == HAL_OK)
  {
      UART_SendString("[PASS] OPMODE_CFG configured\r\n");
  }
  else
  {
      UART_SendString("[FAIL] Failed to configure OPMODE_CFG\r\n");
  }

  UART_SendString("TLA2528 configured successfully\r\n");

  /* 运行I2C通信调试测试 */
  I2C_DebugTest();

  /* 配置MCO引脚输出系统时钟（用于示波器测量） */
  /* PA8作为MCO1引脚 */
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  __HAL_RCC_GPIOA_CLK_ENABLE();
  GPIO_InitStruct.Pin = GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  
  /* 配置MCO1输出系统时钟，不分频 */
  HAL_RCC_MCOConfig(RCC_MCO1, RCC_MCO1SOURCE_SYSCLK, RCC_MCODIV_1);

  /* 启动UART中断接收 */
  rx_index = 0;
  HAL_UART_Receive_IT(&huart1, &rx_buffer[rx_index], 1);
  
  /* 启动USART2中断接收 */
  extern uint8_t rx_buffer_usart2[];
  extern volatile uint8_t rx_index_usart2;
  rx_index_usart2 = 0;
  HAL_UART_Receive_IT(&huart2, &rx_buffer_usart2[rx_index_usart2], 1);

//  /* 打印初始化信息 */
//  UART_SendString("\r\n=== TEC Controller Comm System Init OK ===\r\n");
//  UART_SendString("[INFO] USART1 (PA9/PA10): 115200 bps\r\n");
//  UART_SendString("[INFO] USART2 (PA2/PA3): 9600 bps\r\n");
//  UART_SendString("[INFO] Link: PC <-> USART1 <-> USART2 <-> TEC\r\n");
//  UART_SendString("[INFO] Cmds: S1,S1A,S1B,S0,SC,SS,T0,T1,RE,RR,SEN0,SEN1,REN,SA,RA,SAL,RAL,SWR,RWR,RP,RP1,RP2,RP3,RS,RS1,RS2,RS3,RD,SP,RFP,SI,RFI,SD,RFD,ST,RT,SRT,RRT,SROC,RROC,SF,RF,SM,RM,SBR,RBR,RV,SAD,RAD\r\n");
//  UART_SendString("[INFO] PWM cmd: xPWMxxx (x=0..3, xxx=off% 000~100). duty=100-off\r\n");
//  UART_SendString("[INFO] Example: S1 25.5\r\n");
//  UART_SendString("==========================================\r\n\r\n");

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  uint8_t ntc_stream_enable = 0;
  uint8_t fg_frq_enable = 0; // FG频率读取和打印使能标志
  uint8_t pwm_id_cfg = 0;
  uint8_t off_percent_cfg = 0;
  uint32_t last_ntc_tick = HAL_GetTick();
  uint32_t last_led_tick = HAL_GetTick();
  uint32_t last_fg_tick = HAL_GetTick();
  uint32_t tec_cmd_send_time = 0; // TEC command send timestamp
  uint8_t tec_wait_response = 0;  // Flag for waiting TEC response
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    /* 轮询SPI NSS状态 */
    SPI2_SlaveComm_Poll();
    
    /* 检查SPI命令 */
    uint8_t spi_cmd[SPI2_MSG_MAX];
    if (SPI2_SlaveComm_HasPendingCmd()) {
      uint8_t spi_cmd_len = SPI2_SlaveComm_GetCmd(spi_cmd, sizeof(spi_cmd));
      if (spi_cmd_len > 0) {
        UART_Printf("[SPI] Received cmd: %s\r\n", spi_cmd);
        
        /* 处理SPI命令 */
        trim_cmd((char *)spi_cmd);
        uint8_t en0 = 0, en1 = 0, a0 = 0, a1 = 0, a2 = 0;
        uint8_t pwm_id = 0, off_percent = 0;
        uint8_t response_buf[SPI2_MSG_MAX];
        uint8_t response_len = 0;
        
        /* NTC_START 指令 */
        if (equals_ignore_case((char *)spi_cmd, "NTC_START")) {
          ntc_stream_enable = 1;
          last_ntc_tick = HAL_GetTick();
          strcpy((char *)response_buf, "[OK] NTC stream ON");
          response_len = strlen((char *)response_buf);
        }
        /* NTC_STOP 指令 */
        else if (equals_ignore_case((char *)spi_cmd, "NTC_STOP")) {
          ntc_stream_enable = 0;
          strcpy((char *)response_buf, "[OK] NTC stream OFF");
          response_len = strlen((char *)response_buf);
        }
        /* FG_FRQ_START 指令 */
        else if (equals_ignore_case((char *)spi_cmd, "FG_FRQ_START")) {
          fg_frq_enable = 1;
          last_fg_tick = HAL_GetTick();
          strcpy((char *)response_buf, "[OK] FG frequency read started");
          response_len = strlen((char *)response_buf);
        }
        /* FG_FRQ_STOP 指令 */
        else if (equals_ignore_case((char *)spi_cmd, "FG_FRQ_STOP")) {
          fg_frq_enable = 0;
          uint32_t time_since_last_capture = HAL_GetTick() - last_fg_update_time;
          if (time_since_last_capture > 2000U) {
            fg_frequency = 0.0f;
          }
          float fan_rpm = fg_frequency * 30.0f;
          snprintf((char *)response_buf, sizeof(response_buf), 
                   "[FG] Freq: %.2f Hz, RPM: %.0f", fg_frequency, fan_rpm);
          response_len = strlen((char *)response_buf);
        }
        /* FGRDxxxx 指令 - 等待捕获完成后返回结果 */
        else if (parse_fgrd_cmd((const char *)spi_cmd, &en0, &en1, &a0, &a1, &a2)) {
          MUX_SetFGMuxPins(en0, en1, a0, a1, a2);
          
          /* 重置捕获统计 */
          fg_capture_count = 0;
          fg_frequency = 0.0f;
          fg_last_time = 0;
          fg_current_time = 0;
          fg_period = 0;
          
          uint32_t fgrd_start_tick = HAL_GetTick();
          uint32_t fgrd_settle_until = fgrd_start_tick + 20U; /* 等模拟开关稳定 */
          uint32_t fgrd_timeout_ms = 2000U;
          
          /* 等待MUX稳定和捕获完成 */
          uint8_t capture_done = 0;
          while ((HAL_GetTick() - fgrd_start_tick) < fgrd_timeout_ms) {
            /* 喂狗 */
            HAL_WWDG_Refresh(&hwwdg);
            
            uint32_t now = HAL_GetTick();
            if (now >= fgrd_settle_until) {
              uint32_t time_since_last_capture = now - last_fg_update_time;
              
              /* 捕获≥2个边沿即可得到有效周期；或者超时也回读一次 */
              if (fg_capture_count >= 2 ||
                  time_since_last_capture > fgrd_timeout_ms ||
                  (now - fgrd_start_tick) > fgrd_timeout_ms) {
                capture_done = 1;
                break;
              }
            }
            HAL_Delay(1);
          }
          
          /* 准备响应 */
          float freq_to_report = fg_frequency;
          if (fg_capture_count < 2) {
            freq_to_report = 0.0f;
          }
          float fan_rpm = freq_to_report * 30.0f;
          
          snprintf((char *)response_buf, sizeof(response_buf),
                   "[FGRD] PA1(TIM2_CH2) MUX(EN0=%u,EN1=%u,A0=%u,A1=%u,A2=%u) Freq: %.2f Hz, RPM: %.0f, Captures: %lu",
                   en0, en1, a0, a1, a2, freq_to_report, fan_rpm, fg_capture_count);
          response_len = strlen((char *)response_buf);
        }
        /* xPWMxxx 指令 */
        else if (parse_xpwm_cmd((const char *)spi_cmd, &pwm_id, &off_percent)) {
          uint8_t duty_on_percent = (uint8_t)(100U - off_percent);
          PWM_SetDutyPercent_ById(pwm_id, duty_on_percent);
          snprintf((char *)response_buf, sizeof(response_buf),
                   "[OK] PWM%u duty=%u%%", pwm_id, duty_on_percent);
          response_len = strlen((char *)response_buf);
        }
        /* 其他指令：转发到USART2 (TEC) */
        else {
          /* 转发命令到USART2 */
          HAL_UART_Transmit(&huart2, spi_cmd, spi_cmd_len, 100);
          uint8_t crlf[] = "\r\n";
          HAL_UART_Transmit(&huart2, crlf, 2, 100);
          
          /* 等待TEC响应（最多500ms） */
          uint32_t wait_start = HAL_GetTick();
          uint8_t tec_response[SPI2_MSG_MAX];
          uint8_t tec_resp_len = 0;
          while ((HAL_GetTick() - wait_start) < 500 && tec_resp_len < sizeof(tec_response) - 1) {
            uint8_t rx_data_usart2[32];
            uint8_t len_usart2 = USART2_GetReceivedData(rx_data_usart2, sizeof(rx_data_usart2));
            if (len_usart2 > 0) {
              if (tec_resp_len + len_usart2 < sizeof(tec_response)) {
                memcpy(&tec_response[tec_resp_len], rx_data_usart2, len_usart2);
                tec_resp_len += len_usart2;
              }
              wait_start = HAL_GetTick(); /* 重置超时 */
            }
            HAL_Delay(1);
          }
          
          if (tec_resp_len > 0) {
            tec_response[tec_resp_len] = '\0';
            strncpy((char *)response_buf, (char *)tec_response, sizeof(response_buf) - 1);
            response_buf[sizeof(response_buf) - 1] = '\0';
            response_len = strlen((char *)response_buf);
          } else {
            strcpy((char *)response_buf, "[ERR] TEC timeout");
            response_len = strlen((char *)response_buf);
          }
        }
        
        /* 设置SPI响应 */
        if (response_len > 0) {
          SPI2_SlaveComm_SetResponse(response_buf, response_len);
          UART_Printf("[SPI] Response: %s\r\n", response_buf);
        }
      }
    }
    
    /* 检查串口接收 - 处理接收到的字符串 */
    uint8_t rx_data[32];
    uint8_t len = USART1_GetReceivedData(rx_data, sizeof(rx_data));
    if (len > 0)
    {
      trim_cmd((char *)rx_data);
      uint8_t en0 = 0, en1 = 0, a0 = 0, a1 = 0, a2 = 0;

      /* NTC_START 指令：开启NTC温度数据回读和打印 */
      if (equals_ignore_case((char *)rx_data, "NTC_START"))
      {
        ntc_stream_enable = 1;
        last_ntc_tick = HAL_GetTick();
        UART_SendString("[OK] NTC stream ON\r\n");
      }
      /* NTC_STOP 指令：停止NTC温度数据回读和打印 */
      else if (equals_ignore_case((char *)rx_data, "NTC_STOP"))
      {
        ntc_stream_enable = 0;
        UART_SendString("[OK] NTC stream OFF\r\n");
      }
      /* FGDEBUG 指令：开启FG捕获debug打印 */
      else if (equals_ignore_case((char *)rx_data, "FGDEBUG"))
      {
        fg_debug_enabled = 1;
        UART_SendString("[OK] FG debug ON\r\n");
      }
      /* FGDEBUGOFF 指令：关闭FG捕获debug打印 */
      else if (equals_ignore_case((char *)rx_data, "FGDEBUGOFF"))
      {
        fg_debug_enabled = 0;
        UART_SendString("[OK] FG debug OFF\r\n");
      }
      /* FGRESET 指令：重置FG捕获计数 */
      else if (equals_ignore_case((char *)rx_data, "FGRESET"))
      {
        fg_capture_count = 0;
        fg_frequency = 0.0f;
        UART_SendString("[OK] FG capture reset\r\n");
      }
      /* FG_FRQ_START 指令：开始FG频率读取和打印 */
      else if (equals_ignore_case((char *)rx_data, "FG_FRQ_START"))
      {
        fg_frq_enable = 1;
        last_fg_tick = HAL_GetTick();
        UART_SendString("[OK] FG frequency read started\r\n");
      }
      /* FG_FRQ_STOP 指令：结束FG频率读取和打印，并回读当前频率 */
      else if (equals_ignore_case((char *)rx_data, "FG_FRQ_STOP"))
      {
        fg_frq_enable = 0;
        // 检查是否有新的FG信号，如果超过2秒没有捕获到，频率设为0
        uint32_t time_since_last_capture = HAL_GetTick() - last_fg_update_time;
        if (time_since_last_capture > 2000U)
        {
            fg_frequency = 0.0f;
        }
        // 计算风扇转速（RPM）：FG频率(Hz) * 60 / 2（假设每转产生2个脉冲）
        float fan_rpm = fg_frequency * 30.0f;
        // 回读并打印当前FG频率
        UART_Printf("[FG] Final PA1(TIM2_CH2) Freq: %.2f Hz, RPM: %.0f, Captures: %lu\r\n",
                    fg_frequency, fan_rpm, fg_capture_count);
        UART_SendString("[OK] FG frequency read stopped\r\n");
      }
      /* FGRDxxxx：切换 MUX 通道并回读一次 */
      else if (parse_fgrd_cmd((const char *)rx_data, &en0, &en1, &a0, &a1, &a2))
      {
        /* 先切换 MUX，再重置FG捕获统计，避免混入旧通道的边沿 */
        MUX_SetFGMuxPins(en0, en1, a0, a1, a2);

        fg_one_shot_active = 1;
        fg_one_shot_start_tick = HAL_GetTick();
        fg_one_shot_settle_until = fg_one_shot_start_tick + 20U; /* 等模拟开关稳定 */

        fg_one_shot_en0 = en0;
        fg_one_shot_en1 = en1;
        fg_one_shot_addr0 = a0;
        fg_one_shot_addr1 = a1;
        fg_one_shot_addr2 = a2;

        /* 重置捕获统计与频率 */
        fg_capture_count = 0;
        fg_frequency = 0.0f;
        fg_last_time = 0;
        fg_current_time = 0;
        fg_period = 0;
        last_fg_update_time = fg_one_shot_start_tick;

        UART_SendString("[OK] FGRD pending capture\r\n");
      }
      /* xPWMoff% 指令：在线配置 PUMP/FAN PWM（off%=xxx, duty=100-off） */
      else if (parse_xpwm_cmd((const char *)rx_data, &pwm_id_cfg, &off_percent_cfg))
      {
        uint8_t duty_on_percent = (uint8_t)(100U - off_percent_cfg);

        PWM_SetDutyPercent_ById(pwm_id_cfg, duty_on_percent);

        uint32_t tim_channel = TIM_CHANNEL_1;
        if (pwm_id_cfg == 1) tim_channel = TIM_CHANNEL_2;
        else if (pwm_id_cfg == 2) tim_channel = TIM_CHANNEL_3;
        else if (pwm_id_cfg == 3) tim_channel = TIM_CHANNEL_4;

        UART_Printf("[OK] xPWM%u off=%u%% duty=%u%% (CCR=%lu/ARR=%lu)\r\n",
                    pwm_id_cfg, off_percent_cfg, duty_on_percent,
                    (uint32_t)__HAL_TIM_GET_COMPARE(&htim3, tim_channel),
                    (uint32_t)__HAL_TIM_GET_AUTORELOAD(&htim3) + 1U);
      }
      /* TEC Controller Command: Forward to USART2 */
      else
      {
        // Print debug info
        uint8_t actual_len = strlen((const char *)rx_data);
        UART_Printf("[TEC TX] Cmd: %s, Len: %d\r\n", rx_data, actual_len);
        
        // Forward command to USART2 (TEC Controller)
        HAL_StatusTypeDef tx_status = HAL_UART_Transmit(&huart2, rx_data, actual_len, 100);
        if (tx_status != HAL_OK) {
          UART_Printf("[TEC TX] Error: %d\r\n", tx_status);
        }
        
        // Send CRLF
        uint8_t crlf[] = "\r\n";
        tx_status = HAL_UART_Transmit(&huart2, crlf, 2, 100);
        if (tx_status != HAL_OK) {
          UART_Printf("[TEC TX] CRLF Error: %d\r\n", tx_status);
        } else {
          UART_Printf("[TEC TX] Sent OK, wait for response...\r\n");
          tec_cmd_send_time = HAL_GetTick();
          tec_wait_response = 1;
        }
      }
    }

    /* Check USART2 RX - Process TEC Controller response */
    uint8_t rx_data_usart2[32];
    uint8_t len_usart2 = USART2_GetReceivedData(rx_data_usart2, sizeof(rx_data_usart2));
    if (len_usart2 > 0)
    {
      // Print debug info with hex dump
      UART_Printf("[TEC RX] Len: %d, Data: %s\r\n", len_usart2, rx_data_usart2);
      UART_SendString("[TEC RX] Hex: ");
      for (uint8_t i = 0; i < len_usart2; i++) {
        UART_Printf("%02X ", rx_data_usart2[i]);
      }
      UART_SendString("\r\n");
      
      // Forward response to USART1 (PC)
      UART_SendString("[TEC] ");
      UART_SendString((const char *)rx_data_usart2);
      UART_SendString("\r\n");
      
      // Clear wait flag
      tec_wait_response = 0;
    }
    
    /* Check TEC response timeout (500ms) */
    if (tec_wait_response && (HAL_GetTick() - tec_cmd_send_time > 500))
    {
      UART_SendString("[TEC RX] Timeout: No response from TEC\r\n");
      tec_wait_response = 0;
    }

    /* FGRDxxxx 一键回读一次：等待捕获到足够边沿后打印 */
    if (fg_one_shot_active)	// 检查单次捕获是否激活
    {
      uint32_t now = HAL_GetTick();
      if (now >= fg_one_shot_settle_until)	// 检查是否已过MUX稳定时间
      {
        uint32_t time_since_last_capture = now - last_fg_update_time;

        /* 捕获≥2个边沿即可得到有效周期；或者超时也回读一次 */
        if (fg_capture_count >= 2 ||
            time_since_last_capture > fg_one_shot_timeout_ms ||
            (now - fg_one_shot_start_tick) > fg_one_shot_timeout_ms)
        {
          float freq_to_report = fg_frequency;
          if (fg_capture_count < 2 || time_since_last_capture > fg_one_shot_timeout_ms)
          {
            freq_to_report = 0.0f;
          }

          float fan_rpm = freq_to_report * 30.0f;
          uint32_t captures_to_report = fg_capture_count;

          UART_Printf("[FGRD] PA1(TIM2_CH2) MUX(EN0=%u,EN1=%u,A0=%u,A1=%u,A2=%u) Freq: %.2f Hz, RPM: %.0f, Captures: %lu\r\n",
                      fg_one_shot_en0, fg_one_shot_en1,
                      fg_one_shot_addr0, fg_one_shot_addr1, fg_one_shot_addr2,
                      freq_to_report, fan_rpm, captures_to_report);

          fg_one_shot_active = 0;
        }
      }
    }

    /* 1s 周期：切换 LED（TIM4） */
    if (HAL_GetTick() - last_led_tick >= 1000U)
    {
      last_led_tick += 1000U;

      static uint32_t led3_compare = 500000;
      static uint32_t led4_compare = 0;

      if (led3_compare == 500000) {
        led3_compare = 0;
        led4_compare = 500000;
      } else {
        led3_compare = 500000;
        led4_compare = 0;
      }

      __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_3, led3_compare);
      __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_4, led4_compare);
    }

    /* 1s 周期：仅读取并打印 NTC（仅在收到 NTC 指令后） */
    if (ntc_stream_enable && (HAL_GetTick() - last_ntc_tick >= 1000U))
    {
      last_ntc_tick += 1000U;

      uint16_t adc_value = 0;
      uint8_t raw[2] = {0};
      HAL_StatusTypeDef st = TLA2528_ReadChannel(NTC_CHANNEL, &adc_value, raw);

      if (st == HAL_OK)
      {
          float v_ntc = (float)adc_value / NTC_ADC_MAX * NTC_VCC;
          float r_ntc = 0.0f;
          if (NTC_VCC - v_ntc > 0.001f)
          {
              r_ntc = (NTC_RREF * v_ntc) / (NTC_VCC - v_ntc);
          }

          UART_Printf("NTC CH%d raw=%u, bytes=[0x%02X,0x%02X], Rntc=%.1f ohm\r\n",
                      NTC_CHANNEL, adc_value, raw[0], raw[1], r_ntc);
      }
      else
      {
          UART_Printf("NTC CH%d read error, status=%d\r\n", NTC_CHANNEL, st);
      }
    }

    /* 1s 周期：打印FG信号频率（仅在收到 FG_FRQ_START 指令后） */
    if (fg_frq_enable && (HAL_GetTick() - last_fg_tick >= 1000U))
    {
      last_fg_tick += 1000U;

      // 检查是否有新的FG信号，如果超过2秒没有捕获到，频率设为0
      uint32_t time_since_last_capture = HAL_GetTick() - last_fg_update_time;
      if (time_since_last_capture > 2000U)
      {
          fg_frequency = 0.0f;
      }

      // 计算风扇转速（RPM）：FG频率(Hz) * 60 / 2（假设每转产生2个脉冲）
      float fan_rpm = fg_frequency * 30.0f;

      UART_Printf("[FG] PA1(TIM2_CH2) Freq: %.2f Hz, RPM: %.0f, Captures: %lu\r\n",
                  fg_frequency, fan_rpm, fg_capture_count);
    }

    HAL_Delay(1);
  }
  /* USER CODE END 3 */
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
#ifdef USE_FULL_ASSERT
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

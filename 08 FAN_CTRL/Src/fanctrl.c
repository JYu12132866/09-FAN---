#include "fanctrl.h"
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "tim.h"
#include "usart.h"

/* 风扇硬件信息配置 */
FAN_INFO fans_info[] = {
    {&htim3, TIM_CHANNEL_1},    // 0: PUMP_PWM
    {&htim3, TIM_CHANNEL_2},    // 1: FAN_PWM1
    {&htim3, TIM_CHANNEL_3},    // 2: FAN_PWM2
    {&htim3, TIM_CHANNEL_4},    // 3: FAN_PWM3
};

int  fan_dbg_mode = 0; //debug 模式，设置为1时进入debug模式，关闭自动控制
float fan_rpm_offset = FAN_OFFSET_NORMAL ;  //风扇pwm百分比偏移量，用于ECO 和 Hi ALT模式;

/* ========== FG捕获相关变量 ========== */
static volatile uint32_t fg_last_time = 0;
static volatile uint32_t fg_current_time = 0;
static volatile uint32_t fg_period = 0;
static volatile float fg_frequency = 0.0f;
static volatile uint32_t last_fg_update_time = 0;
static volatile uint32_t fg_capture_count = 0;
static volatile uint8_t fg_debug_enabled = 0;

/* 单次捕获状态 */
static uint8_t fg_one_shot_active = 0;
static uint32_t fg_one_shot_start_tick = 0;
static uint32_t fg_one_shot_settle_until = 0;
static uint8_t fg_one_shot_en0 = 0;
static uint8_t fg_one_shot_en1 = 0;
static uint8_t fg_one_shot_addr0 = 0;
static uint8_t fg_one_shot_addr1 = 0;
static uint8_t fg_one_shot_addr2 = 0;

/**
 * @brief 初始化风扇控制
 * 
 */
void fanctrl_init(void)
{
    // 启动 TIM3 PWM 通道
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_4);
    
    // 上电/复位后：所有风扇/水泵 PWM 置为 0%（低电平）
    fan_pwm_set_all(0);
    
    // 初始化FG信号捕获
    FG_Capture_Init();

}

/**
 * @brief 计算风扇PWM值
 * 
 * @param duty_percent 占空比(0-100)
 * @return uint32_t 计算后的PWM值
 */
static uint32_t calc_fan_pwm_value(uint8_t duty_percent)
{
    uint32_t arr = __HAL_TIM_GET_AUTORELOAD(&htim3);
    uint32_t compare = ((arr + 1U) * duty_percent) / 100U;
    if (compare > arr)
    {
        compare = arr;
    }
    return compare;
}

/**
 * @brief 风扇自动控制逻辑
 * 
 * @param temp_array 获取到的温度数组指针
 * @param temp_num   温度数量
 */
void fan_pwm_auto_set(volatile float* temp_array, int temp_num)
{
    if(fan_dbg_mode)
        return;
    
    // 这里可以根据温度实现自动控制逻辑
    // 目前暂时实现为固定转速
    fan_pwm_set_all(50); // 设置所有风扇为50%占空比
}

/**
 * @brief 根据工作模式，调整风扇转速偏移量
 * 
 */
void update_fanctl_offset(int offset )
{
    fan_rpm_offset = offset;
}

/**
 * @brief 控制单个风扇转速
 * 
 * @param FAN_PWM   风扇pwm占空比(0-100)
 * @param fan_idx   风扇索引号 0 - 3
 */
void fan_pwm_set_single(uint8_t FAN_PWM , int fan_idx)
{
    if(fan_idx < 0 || fan_idx >= sizeof(fans_info)/sizeof(FAN_INFO))
        return;
    
    // 限制占空比范围
    if(FAN_PWM < FAN_PWM_FLOOR)
        FAN_PWM = FAN_PWM_FLOOR;
    else if(FAN_PWM > FAN_PWM_CEIL)
        FAN_PWM = FAN_PWM_CEIL;
    
    uint32_t pwm_value = calc_fan_pwm_value(FAN_PWM);
    __HAL_TIM_SET_COMPARE(fans_info[fan_idx].pwm_tim_hdl, fans_info[fan_idx].pwm_tim_ch, pwm_value);
}

/**
 * @brief 进入风扇Debug模式，自动控制关闭
 * 
 */
void fan_enter_dbg_mode(void)
{
    fan_dbg_mode = 1;
}

/**
 * @brief 退出风扇Debug模式，自动控制开启
 * 
 */
void fan_exit_dbg_mode(void)
{
    fan_dbg_mode = 0;
}

/**
 * @brief 设置所有风扇转速
 * 
 * @param duty_percent 占空比(0-100)
 */
void fan_pwm_set_all(uint8_t duty_percent)
{
    for(int i = 0; i < sizeof(fans_info)/sizeof(FAN_INFO); i++)
    {
        fan_pwm_set_single(duty_percent, i);
    }
}

/* ========== FG捕获相关函数实现 ========== */

/**
 * @brief  TIM2输入捕获回调函数
 * @note   处理FG信号捕获
 */
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    extern volatile uint32_t fg_last_time;
    extern volatile uint32_t fg_current_time;
    extern volatile uint32_t fg_period;
    extern volatile float fg_frequency;
    extern volatile uint32_t last_fg_update_time;
    extern volatile uint32_t fg_capture_count;
    extern volatile uint8_t fg_debug_enabled;
    
    if (htim->Instance == TIM2 && htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)
    {
        static uint32_t callback_count = 0;
        callback_count++;
            
        uint32_t captured_value = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2);

        // 打印回调信息（前几次）
        if (fg_debug_enabled && callback_count <= 5)
        {
            printf("[FG DEBUG] Callback #%lu: TIM2_CH2, Value=%lu\r\n",
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
                printf("[FG DEBUG] First capture: %lu\r\n", captured_value);
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
            printf("[FG DEBUG] Capture #%lu: curr=%lu, last=%lu, period=%lu, freq=%.2f Hz\r\n",
                        fg_capture_count, fg_current_time, fg_last_time, fg_period, fg_frequency);
        }

        // 更新最后捕获时间
        last_fg_update_time = HAL_GetTick();
    }
}

/**
 * @brief  初始化FG信号捕获
 */
HAL_StatusTypeDef FG_Capture_Init(void)
{
    /* 初始化FG信号更新时间 */
    last_fg_update_time = HAL_GetTick();
    
    /* 启动TIM2输入捕获中断（PA1 - TIM2_CH2） */
    HAL_StatusTypeDef status = HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_2);
    
    if (status != HAL_OK)
    {
        printf("[ERROR] Failed to start TIM2 IC interrupt\r\n");
    }
    else
    {
        printf("[INFO] TIM2 CH2 (PA1) input capture started\r\n");
    }
    
    return status;
}

/**
 * @brief  配置CD4051 MUX引脚
 */
void MUX_SetFGMuxPins(uint8_t en0, uint8_t en1,
                      uint8_t addr0, uint8_t addr1, uint8_t addr2)
{
    HAL_GPIO_WritePin(MUX_EN0_GPIO_Port, MUX_EN0_Pin, en0 ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MUX_EN1_GPIO_Port, MUX_EN1_Pin, en1 ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MUX_ADDR0_GPIO_Port, MUX_ADDR0_Pin, addr0 ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MUX_ADDR1_GPIO_Port, MUX_ADDR1_Pin, addr1 ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MUX_ADDR2_GPIO_Port, MUX_ADDR2_Pin, addr2 ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

/**
 * @brief  使能/禁用FG调试打印
 */
void FG_SetDebugEnable(uint8_t enable)
{
    fg_debug_enabled = enable;
}

/**
 * @brief  获取FG调试使能状态
 */
uint8_t FG_GetDebugEnable(void)
{
    return fg_debug_enabled;
}

/**
 * @brief  重置FG捕获计数
 */
void FG_ResetCapture(void)
{
    fg_capture_count = 0;
    fg_frequency = 0.0f;
    fg_last_time = 0;
    fg_current_time = 0;
    fg_period = 0;
}

/**
 * @brief  获取当前FG频率
 */
float FG_GetFrequency(void)
{
    // 检查是否超时
    if (FG_IsTimeout(FG_CAPTURE_TIMEOUT_MS))
    {
        fg_frequency = 0.0f;
    }
    return fg_frequency;
}

/**
 * @brief  获取当前捕获计数
 */
uint32_t FG_GetCaptureCount(void)
{
    return fg_capture_count;
}

/**
 * @brief  获取最后一次捕获时间
 */
uint32_t FG_GetLastUpdateTime(void)
{
    return last_fg_update_time;
}

/**
 * @brief  计算风扇RPM
 */
float FG_CalculateRPM(void)
{
    float freq = FG_GetFrequency();
    // FG频率(Hz) * 60 / 2（假设每转产生2个脉冲）
    return freq * 30.0f;
}

/**
 * @brief  检查FG信号是否超时
 */
uint8_t FG_IsTimeout(uint32_t timeout_ms)
{
    return (HAL_GetTick() - last_fg_update_time) > timeout_ms;
}

/**
 * @brief  打印FG状态信息
 */
void FG_PrintStatus(void)
{
    float freq = FG_GetFrequency();
    float rpm = FG_CalculateRPM();
    printf("[FG] PA1(TIM2_CH2) Freq: %.2f Hz, RPM: %.0f, Captures: %lu\r\n",
           freq, rpm, fg_capture_count);
}

/**
 * @brief  FG捕获主处理函数（在main循环中调用）
 * @note   处理单次捕获逻辑
 */
void FG_Process(void)
{
    if (!fg_one_shot_active)
    {
        return;
    }
    
    uint32_t now = HAL_GetTick();
    if (now < fg_one_shot_settle_until)
    {
        return; // 等待MUX稳定
    }
    
    uint32_t time_since_last_capture = now - last_fg_update_time;
    
    // 捕获≥2个边沿即可得到有效周期；或者超时也回读一次
    if (fg_capture_count >= 2 ||
        time_since_last_capture > FG_CAPTURE_TIMEOUT_MS ||
        (now - fg_one_shot_start_tick) > FG_CAPTURE_TIMEOUT_MS)
    {
        fg_one_shot_active = 0; // 单次捕获完成
    }
}

/**
 * @brief  启动单次FG捕获
 */
void FG_StartOneShot(uint8_t en0, uint8_t en1, 
                     uint8_t addr0, uint8_t addr1, uint8_t addr2)
{
    // 切换MUX
    MUX_SetFGMuxPins(en0, en1, addr0, addr1, addr2);
    
    // 保存配置
    fg_one_shot_en0 = en0;
    fg_one_shot_en1 = en1;
    fg_one_shot_addr0 = addr0;
    fg_one_shot_addr1 = addr1;
    fg_one_shot_addr2 = addr2;
    
    // 重置捕获统计
    FG_ResetCapture();
    last_fg_update_time = HAL_GetTick();
    
    // 启动单次捕获
    fg_one_shot_active = 1;
    fg_one_shot_start_tick = HAL_GetTick();
    fg_one_shot_settle_until = fg_one_shot_start_tick + FG_MUX_SETTLE_MS;
}

/**
 * @brief  检查单次捕获是否完成并获取结果
 */
uint8_t FG_GetOneShotResult(float *freq, float *rpm, uint32_t *captures)
{
    if (fg_one_shot_active)
    {
        return 0; // 尚未完成
    }
    
    float f = FG_GetFrequency();
    if (fg_capture_count < 2 || FG_IsTimeout(FG_CAPTURE_TIMEOUT_MS))
    {
        f = 0.0f;
    }
    
    if (freq) *freq = f;
    if (rpm) *rpm = f * 30.0f;
    if (captures) *captures = fg_capture_count;
    
    return 1;
}

/**
 * @brief  获取单次捕获的MUX配置
 * @note   用于打印调试信息
 */
void FG_GetOneShotConfig(uint8_t *en0, uint8_t *en1, 
                         uint8_t *addr0, uint8_t *addr1, uint8_t *addr2)
{
    if (en0) *en0 = fg_one_shot_en0;
    if (en1) *en1 = fg_one_shot_en1;
    if (addr0) *addr0 = fg_one_shot_addr0;
    if (addr1) *addr1 = fg_one_shot_addr1;
    if (addr2) *addr2 = fg_one_shot_addr2;
}

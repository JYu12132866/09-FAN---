#ifndef _FANCTRL_H
#define _FANCTRL_H

#include "tim.h"
#include "gpio.h"
#include <stdint.h>

#define FAN_OFFSET_NORMAL   0
#define FAN_OFFSET_ECO      -15
#define FAN_OFFSET_HI_ALT   15 

#define FAN_PWM_FLOOR 0
#define FAN_PWM_CEIL  100

/* FG捕获相关定义 */
#define FG_CAPTURE_TIMEOUT_MS   2000U   /* FG捕获超时时间 */
#define FG_MUX_SETTLE_MS        20U     /* MUX稳定时间 */

/**
 * @brief 风扇信息结构体定义
 * 
 */
typedef struct 
{
    TIM_HandleTypeDef*  pwm_tim_hdl;
    uint32_t            pwm_tim_ch; 
}FAN_INFO;

/**
 * @brief 风扇自动控制曲线,分段参数结构体定义
 * 
 */
typedef struct 
{
    float floor_temp;   //曲线分段拐点温度下限
    float ceil_temp;    //曲线分段拐点温度上限
    float k;            //曲线斜率
    float b;            //曲线截距
}CURVE_SECTION_PARAM;

/**
 * @brief 风扇自动控制曲线,分段参数结构体定义
 * 
 */
typedef struct 
{
    int section_num;    //曲线分段数量,最大5段
    CURVE_SECTION_PARAM section_param[5]; //曲线分段参数
}CURVE_PARAM;

/**
 * @brief 风扇组信息结构体定义
 * 
 */
typedef struct  
{
    int curve_idx;              //风扇控制曲线索引号
    int temp_sensor_num;        //需要依赖的温度传感器数量
    int temp_sensor_ch[6];      //需要依赖的温度传感器通道索引
    int fan_num;                //被控风扇通道数量
    int fan_idx[4];             //被控风扇通道索引号
}FAN_GROUP;

extern float fan_rpm_offset;

/**
 * @brief 初始化风扇控制
 * 
 */
void fanctrl_init(void);

/**
 * @brief 风扇自动控制逻辑
 * 
 * @param temp_array 获取到的温度数组指针
 * @param temp_num   温度数量
 */
void fan_pwm_auto_set(volatile float* temp_array, int temp_num);

/**
 * @brief 根据工作模式，调整风扇转速偏移量
 * 
 */
void update_fanctl_offset(int offset );

/**
 * @brief 控制单个风扇转速
 * 
 * @param FAN_PWM   风扇pwm占空比(0-100)
 * @param fan_idx   风扇索引号 0 - 3
 */
void fan_pwm_set_single(uint8_t FAN_PWM , int fan_idx);

/**
 * @brief 进入风扇Debug模式，自动控制关闭
 * 
 */
void fan_enter_dbg_mode(void);

/**
 * @brief 退出风扇Debug模式，自动控制开启
 * 
 */
void fan_exit_dbg_mode(void);

/**
 * @brief 设置所有风扇转速
 * 
 * @param duty_percent 占空比(0-100)
 */
void fan_pwm_set_all(uint8_t duty_percent);

/* ========== FG捕获相关函数 ========== */

/**
 * @brief  初始化FG信号捕获（包括TIM2输入捕获中断）
 * @retval HAL_StatusTypeDef
 */
HAL_StatusTypeDef FG_Capture_Init(void);

/**
 * @brief  配置CD4051 MUX引脚
 * @param  en0: EN0电平
 * @param  en1: EN1电平
 * @param  addr0: ADDR0电平
 * @param  addr1: ADDR1电平
 * @param  addr2: ADDR2电平
 * @retval None
 */
void MUX_SetFGMuxPins(uint8_t en0, uint8_t en1, uint8_t addr0, uint8_t addr1, uint8_t addr2);

/**
 * @brief  选择CD4051 Y4通道（FAN11）
 * @retval None
 */
void MUX_SelectY4(void);

/**
 * @brief  使能FG调试打印
 * @param  enable: 1=使能, 0=禁用
 * @retval None
 */
void FG_SetDebugEnable(uint8_t enable);

/**
 * @brief  获取FG调试使能状态
 * @retval uint8_t: 1=使能, 0=禁用
 */
uint8_t FG_GetDebugEnable(void);

/**
 * @brief  重置FG捕获计数
 * @retval None
 */
void FG_ResetCapture(void);

/**
 * @brief  获取当前FG频率
 * @retval float: 频率(Hz)
 */
float FG_GetFrequency(void);

/**
 * @brief  获取当前捕获计数
 * @retval uint32_t: 捕获次数
 */
uint32_t FG_GetCaptureCount(void);

/**
 * @brief  获取最后一次捕获时间
 * @retval uint32_t: tick值
 */
uint32_t FG_GetLastUpdateTime(void);

/**
 * @brief  计算风扇RPM
 * @retval float: RPM值
 */
float FG_CalculateRPM(void);

/**
 * @brief  检查FG信号是否超时
 * @param  timeout_ms: 超时时间(ms)
 * @retval uint8_t: 1=超时, 0=未超时
 */
uint8_t FG_IsTimeout(uint32_t timeout_ms);

/**
 * @brief  打印FG状态信息
 * @retval None
 */
void FG_PrintStatus(void);

/**
 * @brief  FG捕获主处理函数（在main循环中调用）
 * @retval None
 */
void FG_Process(void);

/**
 * @brief  启动单次FG捕获
 * @param  en0: EN0电平
 * @param  en1: EN1电平
 * @param  addr0: ADDR0电平
 * @param  addr1: ADDR1电平
 * @param  addr2: ADDR2电平
 * @retval None
 */
void FG_StartOneShot(uint8_t en0, uint8_t en1, uint8_t addr0, uint8_t addr1, uint8_t addr2);

/**
 * @brief  检查单次捕获是否完成并获取结果
 * @param  freq: 频率输出指针
 * @param  rpm: RPM输出指针
 * @param  captures: 捕获次数输出指针
 * @retval uint8_t: 1=完成, 0=未完成
 */
uint8_t FG_GetOneShotResult(float *freq, float *rpm, uint32_t *captures);

#endif /* _FANCTRL_H */

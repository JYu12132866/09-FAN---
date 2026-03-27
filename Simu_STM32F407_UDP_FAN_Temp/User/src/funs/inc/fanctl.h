#ifndef _FANCTL_H
#define _FANCTL_H

#include "tim.h"
#include <stdint.h>

#define FAN_OFFSET_NORMAL   0
#define FAN_OFFSET_ECO      -15
#define FAN_OFFSET_HI_ALT   15 

extern float fan_rpm_offset;

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


typedef struct 
{
    TIM_HandleTypeDef*  pwm_tim_hdl;
    uint32_t            pwm_tim_ch; 
}FAN_INFO;


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
 * @param FAN_PWM   风扇pwm pulse thrushold
 * @param fan_idx   风扇索引号 0 - 9 
 */
void fan_pwm_set_single(uint16_t FAN_PWM , int fan_idx);

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


#endif /* _FANCTL_H */

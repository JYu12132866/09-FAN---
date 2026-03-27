#include "fanctl.h"
#include <stdint.h>
#include "tim.h"
#include "logger.h"
#include "modectl.h"
// #include "err.h"

#define FAN_PWM_FLOOR 40
#define FAN_PWM_CEIL  100
#define FAN_PWM_FIX_NORMAL  70
#define FAN_PWM_FIX_HI_ALT  100

/*风扇timer与硬件映射关系===================================================================================================================  
|                                  |              NP52SL (待完善)    |                              LP92SLB                             |
| Fan  | timer |       channel     |                                 |     原理图线号    |     PCB接口     |            风扇             |
   1     htim1      TIM_CHANNEL_1              FAN1/FAN5                    FAN1_PWM          FAN1/FAN5         激光器风扇1/环境风扇1    
   2     htim1      TIM_CHANNEL_2                no use                     FAN2_PWM            -                       -          
   3     htim2      TIM_CHANNEL_1                no use                     FAN3_PWM            -                       -
   4     htim2      TIM_CHANNEL_2                no use                     FAN4_PWM            -                       -
   5     htim4      TIM_CHANNEL_4                no use                     FAN5_PWM            -                       -
   6     htim3      TIM_CHANNEL_1                no use                     FAN6_PWM        FAN_L1/FAN_L2               -    
   7     htim3      TIM_CHANNEL_2         conflict with motor3_step         FAN7_PWM        FAN_W1/FAN_W2       激光器风扇3/成像壳体风扇
   8     htim1      TIM_CHANNEL_3               FAN                         FAN8_PWM        FAN_L3/FAN_L4       DMD风扇/环境风扇2
   9     htim1      TIM_CHANNEL_4                  √                        FAN9_PWM        FAN_B1/FAN_B2       激光器风扇2/-
   10    htim4      TIM_CHANNEL_2                no use                     FAN10_PWM           -                       -
*/

/*风扇位置示意图===========================================================================================
a. 环境风扇1 
b. 环境风扇2
c. 激光器风扇1
d. 激光器风扇2
e. 激光器风扇3
f. 成像壳体风扇
g. DMD风扇
       \  /
       _\/_
______|    |_______
|        f       g|
|                 |
|e               b|
|d                |
|c               a|
|_________________|
*/

//=============================================================== 
/*所有风扇通道timer/channel映射关系表*/
FAN_INFO fans_info[] = {
    {&htim1, TIM_CHANNEL_1},    //Fan_1
    {&htim1, TIM_CHANNEL_2},    //Fan_2
    {&htim2, TIM_CHANNEL_1},    //Fan_3
    {&htim2, TIM_CHANNEL_2},    //Fan_4
    {&htim4, TIM_CHANNEL_4},    //Fan_5  
    {&htim3, TIM_CHANNEL_1},    //Fan_6
    {&htim3, TIM_CHANNEL_2},    //Fan_7
    {&htim1, TIM_CHANNEL_3},    //Fan_8
    {&htim1, TIM_CHANNEL_4},    //Fan_9
    {&htim4, TIM_CHANNEL_2},    //Fan_10
};

//风扇控制曲线参数=================================================
/*
函数说明：
    y = kx + b 线性关系，根据温度计算PWM占空比，分段自动控制
    x :温度
    y :风扇PWM占空比
分段拐点： 
    x: 20.00, 40.00, 60.00, 80.00
    y: 30.00, 40.00, 60.00, 100.00
*/
//风扇控制曲线1参数{x,y,k,b}
CURVE_PARAM curve1 = {
    .section_num = 5,
    .section_param[0] = {-99.00, 30.00 ,0.00, FAN_PWM_FLOOR},       //y = 40                -99 <= x < 30
    .section_param[1] = {30.00, 40.00, 1.00, 10.00},                //y = x + 10             30 <= x < 40
    .section_param[2] = {40.00, 60.00, 1.00, 10.00},                //y = x + 10             40 <= x < 60
    .section_param[3] = {60.00, 80.00, 1.50, -20.00},               //y = 1.5x - 20          60 <= x < 80
    .section_param[4] = {80.00, 199.00, 0.00, FAN_PWM_CEIL},        //y = 100                80 <= x < 199
};
//风扇控制曲线2参数{x,y,k,b}
CURVE_PARAM curve2 = {
    .section_num = 5,
    .section_param[0] = {-99.00, 30.00 ,0.00, FAN_PWM_FLOOR + 10},       //y =50                -99 <= x < 30
    .section_param[1] = {30.00, 40.00, 1.00, 20.00},                //y = x + 20             30 <= x < 40
    .section_param[2] = {40.00, 60.00, 1.00, 20.00},                //y = x + 20             40 <= x < 60
    .section_param[3] = {60.00, 80.00, 1.50, -10.00},               //y = 1.5x - 10          60 <= x < 80
    .section_param[4] = {80.00, 199.00, 0.00, FAN_PWM_CEIL},        //y = 100                80 <= x < 199
};

//风扇控制曲线表
CURVE_PARAM* curve_tbl[] = {
    &curve1,      
    &curve2,      
};

/* 风扇控制曲线-温度传感器-风扇组绑定关系表 */
FAN_GROUP fan_grp_tbl[] = {
    {1,   4, {2,3,4,6},    3, {0,8,7}},       //组0,使用控制曲线1， 4个温度传感器，3个风扇通道     G1 G2 B1 B2    -> Fan_1/Fan_9/Fan_8 (激光器风扇1/环境风扇1/激光器风扇2/DMD风扇 共4个风扇)
    {0,   2, {0, 1, },     2, {5,6}},         //组1,使用控制曲线2， 2个温度传感器，1个风扇通道     R1 R2          -> Fan_6/Fan_7 (环境风扇2/激光器风扇3/成像壳体风扇 共3个风扇) 
};

//===================================================================

int  fan_dbg_mode = 0; //debug 标记，若为1，则进入debug模式，关闭自动控制
float fan_rpm_offset = FAN_OFFSET_NORMAL ;  //风扇pwm百分比偏移量，用于ECO 和 Hi ALT模式;

/**
 * @brief 通过温度组的最大温度，计算该组风扇所需的pwm值
 * 
 * @param max_temp 
 * @return int 
 */
inline int calc_fan_pwm(float max_temp,  CURVE_PARAM* curve)
{

	int curve_section_num  = curve->section_num;

    int fan_pwm =0 ;
    for (size_t i = 0; i < curve_section_num ; i++)
    {
        if(max_temp >= curve->section_param[i].floor_temp && max_temp < curve->section_param[i].ceil_temp){
            fan_pwm =  (curve->section_param[i].k  * max_temp + (curve->section_param[i].b) + fan_rpm_offset); //计算得出的pwm值（百分比）
			break;
        }
    }
    //限定上下限，不能低于40%和高于100%，防止因为高海拔和节能模式导致风扇停转或者全速转动
    if(fan_pwm < FAN_PWM_FLOOR)
        fan_pwm = FAN_PWM_FLOOR;
    else if(fan_pwm > FAN_PWM_CEIL)
        fan_pwm = FAN_PWM_CEIL;
    fan_pwm = Tim_PWM_DC_end * fan_pwm / 100; //转换成实际PWM参数返回并给到TImer
    return fan_pwm;
} 


void fan_pwm_auto_set(volatile float* temp_array, int temp_num)
{
    if(fan_dbg_mode)
        return;
// #if(Projector_Model == LP92SLB)   //LP92SLB 风扇组分区控制
#if(Projector_Model == Non_Model)
    log_info("Auto fan ctl:0x%x",work_mode);
    const static int group_num = sizeof(fan_grp_tbl)/sizeof(FAN_GROUP);
    //遍历风扇组
    for (size_t i = 0; i < group_num; i++)
    {
        float max_temp = 0;
        int fan_pwm= 0;
        //组内计算最大温度
        for (size_t j = 0; j < fan_grp_tbl[i].temp_sensor_num ; j++)
        {   
            int temp_idx =fan_grp_tbl[i].temp_sensor_ch[j]; 
            if(temp_array[temp_idx] > max_temp)
                max_temp = temp_array[temp_idx];
        }
        //获取对应曲线参数指针， 计算改组对应对应pwm
        CURVE_PARAM* curve = curve_tbl[fan_grp_tbl[i].curve_idx];
        fan_pwm = calc_fan_pwm(max_temp, curve);
        //该组内风扇全部更新转速
        for (size_t j = 0; j < fan_grp_tbl[i].fan_num; j++)
        {
            int fan_idx = fan_grp_tbl[i].fan_idx[j];
            __HAL_TIM_SET_COMPARE(fans_info[fan_idx].pwm_tim_hdl, fans_info[fan_idx].pwm_tim_ch, fan_pwm); 
        }
        log_info("Fan group %d Max_temp: %.2f℃, Fan_pwm %.2f%%", i, max_temp, (float)(fan_pwm * 100 / Tim_PWM_DC_end) );
    }
#else//其它型号暂时采用全局控制，不单独控制风扇转速
    log_info("Fixed fan ctl:0x%x", work_mode);
    uint16_t fan_pwm = 0;
    if(work_mode == WORK_MODE_HI_ALT)
        fan_pwm = Tim_PWM_DC_end * FAN_PWM_FIX_HI_ALT / 100;
    else
        fan_pwm =  Tim_PWM_DC_end * FAN_PWM_FIX_NORMAL / 100;

    FAN_PWM_Set(fan_pwm);
#endif
}

inline void update_fanctl_offset(int offset )
{
    fan_rpm_offset = offset;
}

//Debug及测试接口，调试使用====================================================================================
void fan_pwm_set_single(uint16_t FAN_PWM , int fan_idx)
{
  __HAL_TIM_SET_COMPARE(fans_info[fan_idx].pwm_tim_hdl, fans_info[fan_idx].pwm_tim_ch , FAN_PWM); 
}

inline void fan_enter_dbg_mode(void)
{
    fan_dbg_mode = 1;
    log_info("FanCtl Enter debug mode");
}

inline void fan_exit_dbg_mode(void)
{
    fan_dbg_mode = 0;
    log_info("FanCtl Exit debug mode");
}

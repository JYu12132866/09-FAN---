#include "modectl.h"
#include "tim.h"
#include "logger.h"
#include "error.h"
#include "fanctl.h"
#include "rt_param.h"

uint8_t work_mode = WORK_MODE_NORMAL;

/**
 * @brief 开启eco模式，若高海拔模式已经开启，则不做风扇设置
 * 
 */
void modectl_eco_mode_on(void)
{
    if(work_mode & WORK_MODE_ECO)
        return; 
    work_mode |= WORK_MODE_ECO;
    rt_param.work_mode=work_mode; //TODO:需要更新删除不同C文件之间的变量调用
    save_rt_param_to_flash();
    if(work_mode & WORK_MODE_HI_ALT)
        return; 
    // FAN_PWM_Set(FAN_PWM_DC_ECO);
    update_fanctl_offset(FAN_OFFSET_ECO);
    return;
}

/**
 * @brief 关闭eco模式，若高海拔模式已经开启，则不做风扇设置
 * 
 */
void modectl_eco_mode_off(void)
{
    if(!(work_mode & WORK_MODE_ECO))
        return; 
    work_mode &= ~WORK_MODE_ECO;
    rt_param.work_mode=work_mode; //TODO:需要更新删除不同C文件之间的变量调用
    save_rt_param_to_flash();
    if(work_mode & WORK_MODE_HI_ALT)
        return; 
    // FAN_PWM_Set(FAN_PWM_DC_NORMAL); 
    update_fanctl_offset(FAN_OFFSET_NORMAL);
    return;
}

/**
 * @brief 开启高海拔模式，风扇设置满速
 * 
 */
void modectl_hi_alt_mode_on(void)
{
    if(work_mode & WORK_MODE_HI_ALT)
        return; 
    work_mode |= WORK_MODE_HI_ALT;
    rt_param.work_mode=work_mode; //TODO:需要更新删除不同C文件之间的变量调用
    save_rt_param_to_flash();
    // FAN_PWM_Set(FAN_PWM_DC_HI_ALT);
    update_fanctl_offset(FAN_OFFSET_HI_ALT);
    return;
}

/**
 * @brief 关闭高海拔模式，若eco模式已经开启，则设置风扇为eco模式
 * 
 */
void modectl_hi_alt_mode_off(void)
{
    if(!(work_mode & WORK_MODE_HI_ALT))
        return; 
    work_mode &= ~WORK_MODE_HI_ALT;
    rt_param.work_mode=work_mode; //TODO:需要更新删除不同C文件之间的变量调用
    save_rt_param_to_flash();
    if(work_mode & WORK_MODE_ECO)
        // FAN_PWM_Set(FAN_PWM_DC_ECO);
        update_fanctl_offset(FAN_OFFSET_ECO);
    else 
        // FAN_PWM_Set(FAN_PWM_DC_NORMAL);
        update_fanctl_offset(FAN_OFFSET_NORMAL);
    return ;
}

void modectl_switch_mode(uint8_t cmd)
{
    switch (cmd)
    {
    case MODECTL_CMD_HI_ALT_OFF:
        modectl_hi_alt_mode_off();
        rt_param.high_altitude = 0;
        log_info("hi alt mode off");
        break;
    case MODECTL_CMD_HI_ALT_ON:
        modectl_hi_alt_mode_on();
        rt_param.high_altitude = 1;
        log_info("hi alt mode on");
        break;
    case MODECTL_CMD_ECO_OFF:
        modectl_eco_mode_off();
        rt_param.eco_mode_en = 0;
        log_info("eco mode off");
        break;
    case MODECTL_CMD_ECO_ON:
        modectl_eco_mode_on();
        rt_param.eco_mode_en = 1;
        log_info("eco mode on");
        break;
    default:
        break;
    }
    save_rt_param_to_flash();
    return;
}

void modectl_init_mode(uint8_t mode)
{
    if(mode > WORK_MODE_ALL_ON )  //获取到的工作模式不合法，则重设为正常模式
    {
        work_mode = WORK_MODE_NORMAL;
        rt_param.work_mode=work_mode; //TODO:需要更新删除不同C文件之间的变量调用
    }
    else
    {
        work_mode = mode;
        // work_mode = WORK_MODE_HI_ALT;
    } 
    if(work_mode & WORK_MODE_HI_ALT)
    {
        log_info("Init to Hi Alt Mode");
        update_fanctl_offset(FAN_OFFSET_HI_ALT);
        // FAN_PWM_Set(FAN_PWM_DC_HI_ALT);
    }
    else if(work_mode & WORK_MODE_ECO)
    {
        log_info("Init to Eco Mode");
        // FAN_PWM_Set(FAN_PWM_DC_ECO);
        update_fanctl_offset(FAN_OFFSET_ECO);
    }
    else 
    {
        log_info("Init to Normal Mode");
        // FAN_PWM_Set(FAN_PWM_DC_NORMAL);
        update_fanctl_offset(FAN_OFFSET_NORMAL);
    }
    return;
}


int modeclt_is_HiAlt_On(void)
{
    return (work_mode & WORK_MODE_HI_ALT);
}


int modeclt_is_ECO_On(void)
{
    return (work_mode & WORK_MODE_HI_ALT);
}

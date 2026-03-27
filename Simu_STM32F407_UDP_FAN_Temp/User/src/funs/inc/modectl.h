#ifndef _MODE_CTL_H
#define _MODE_CTL_H
#include <stdint.h>
   
/* 节能模式 和 高海拔模式 可以同时开启;
*  高海拔模式优先级更高，如果高海拔模式开启，节能模式不起作用;
*  高海拔模式退出后，节能模式恢复生效;
*  按bit位定义工作模式，后期可增加
*  |NULL|NULL|NULL|NULL|NULL|NULL| HI_ALT | ECO |
*    7    6    5    4    3    2       1      0
*/
#define WORK_MODE_NORMAL    0x00    //普通模式
#define WORK_MODE_ECO       0x01    //节能模式
#define WORK_MODE_HI_ALT    0x02    //高海拔模式
#define WORK_MODE_ALL_ON    0x03    //所有模式开启

#define MODECTL_CMD_HI_ALT_OFF  0X60
#define MODECTL_CMD_HI_ALT_ON   0X61
#define MODECTL_CMD_ECO_OFF     0X62 
#define MODECTL_CMD_ECO_ON      0X63

extern uint8_t work_mode;

/**
 * @brief 工作模式切换函数，根据cmd进行切换，切换模式后，会自动设置风扇转速
 * 
 */
void modectl_switch_mode(uint8_t cmd);

/**
 * @brief 工作模式初始化函数，根据Flash中的数据进行初始化风扇转速。
 * 
 */
void modectl_init_mode(uint8_t mode);


int modeclt_is_HiAlt_On(void);

int modeclt_is_ECO_On(void);


#endif //_MODE_CTL_H

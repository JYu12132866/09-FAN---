#pragma once 
#include "stdint.h"

/**
 * @brief 设置3D同步延迟参数
 * @param value 
 * @return 
 */
int set_sync_3d_delay_arg(uint8_t value);


/**
 * @brief 应用3D同步延迟参数
 * @param  
 * @return 
 */
int apply_sync_3d_delay_arg(void);

int AD5241_write_value(uint8_t value);

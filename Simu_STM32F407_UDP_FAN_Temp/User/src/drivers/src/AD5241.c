#include "../inc/AD5241.h"
#include "utils/inc/logger.h"
#include "rt_param.h" 
#include "i2c.h"
#include "../utils/inc/logger.h"

#define AD5241_I2C_ADDR         0x5f
#define AD5241_I2C_CMD_WRITE    0x18
#define AD5241_I2C_HANDLE       hi2c2
#define AD5241_I2C_TIMEOUT      1000

int AD5241_write_value(uint8_t value)
{
    uint8_t data[2] = {AD5241_I2C_CMD_WRITE, value};
    HAL_StatusTypeDef ret ;
    ret =  HAL_I2C_Master_Transmit(&AD5241_I2C_HANDLE, AD5241_I2C_ADDR, data, 2, AD5241_I2C_TIMEOUT);
    if(ret != HAL_OK)
    {
        switch(ret)
        {
            case HAL_ERROR:
                log_err("AD5241 write value error\n");
                break;
            case HAL_BUSY:
                log_err("AD5241 write value busy\n");
                break;
            case HAL_TIMEOUT:
                log_err("AD5241 write value timeout\n");
                break;
            default:
                log_err("AD5241 write value unknown error\n");
                break;
        }   
        return -1;
    }
    return 0;
}


int apply_sync_3d_delay_arg(void)
{
    AD5241_write_value(rt_param.sync_3d_delay_arg);
    log_info("3d sync delay set to %d\n", rt_param.sync_3d_delay_arg);
    return 0;
}


int set_sync_3d_delay_arg(uint8_t value)
{
    rt_param.sync_3d_delay_arg = value;
    save_rt_param_to_flash();
    apply_sync_3d_delay_arg();
    return 0;
}

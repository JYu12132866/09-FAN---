/**
 * @file platform inf.c
 * @author Primo Wu (wuxinyu@sae118.net)
 * @brief Platform I/O functions, Applicate to stm32f407 platform  
 * @version 0.1
 * @date 2023-09-15
 * 
 * @copyright Copyright (c) 2023 Smartsens.
 * 
 */
#include "platform.h"
#include "stm32f4xx_hal.h"

extern I2C_HandleTypeDef hi2c2;


/**
 * @brief it6635 u8 reg write function
 * 
 * @param addr  slave address
 * @param offset reg addr 
 * @param length data len
 * @param buffer buff ptr
 * @return u8 
 */
u8 it6635_i2c_write(u8 addr, u8 offset, u8 length, u8 *buffer)
{
    int result;
    result = HAL_I2C_Mem_Write(&hi2c2, addr, offset, 1 , buffer, length, 1000);
    if ( result == HAL_OK )
    {
        
        return 1;
    }
    else
    {
        printf("ERROR: it6635_i2c_write %02x %02x %02x, result: %d\r\n", addr, offset, *buffer, result);
        HAL_Delay(100);
    }
    return 0;
}

/**
 * @brief it6635 u8 reg read function
 * 
 * @param addr  slave address
 * @param offset reg addr 
 * @param length data len
 * @param buffer buff ptr
 * @return u8 
 */
u8 it6635_i2c_read(u8 addr, u8 offset, u8 length, u8 *buffer)
{
    int result;
    result = HAL_I2C_Mem_Read(&hi2c2, addr, offset, 1 , buffer, length, 1000);
    if ( result == HAL_OK )
    {
        return 1;
    }
    else
    {
        printf("ERROR: it6635_i2c_read %02x %02x %02x, result: %d\r\n", addr, offset, length, result);
        HAL_Delay(100);
    }
    return 0;
}

/**
 * @brief Get the tick count object
 * 
 * @return __tick 
 */
__tick get_tick_count(void)
{
	return HAL_GetTick();	
}

/**
 * @brief delys 1ms
 * 
 * @param ms 
 */
void delay1ms(u16 ms)
{
    HAL_Delay(ms);
}

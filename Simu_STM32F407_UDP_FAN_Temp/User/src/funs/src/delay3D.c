#include <math.h>
#include <stdint.h>
#include <stdbool.h>
#include "delay3D.h"
#include "AD5241.h"
#include "rt_param.h"
#include "logger.h"



//LTC6994 mode selection function: set DIVCODE mode
void select_LTC6994_divcode_mode(uint8_t mode)
{
    // Set ADG704 GPIO selection mode according to mode
    if(mode == 3)
    {
        HAL_GPIO_WritePin(GPIOF, GPIO_PIN_6, GPIO_PIN_RESET);       // 0
        HAL_GPIO_WritePin(GPIOF, GPIO_PIN_5, GPIO_PIN_RESET);       // 0
    }
    else if(mode == 12)
    {
        HAL_GPIO_WritePin(GPIOF, GPIO_PIN_6, GPIO_PIN_RESET);       // 0
        HAL_GPIO_WritePin(GPIOF, GPIO_PIN_5, GPIO_PIN_SET);         // 1
    }
    else if(mode == 2)
    {
        HAL_GPIO_WritePin(GPIOF, GPIO_PIN_6, GPIO_PIN_SET);         // 1
        HAL_GPIO_WritePin(GPIOF, GPIO_PIN_5, GPIO_PIN_RESET);       // 0
    }
    else if(mode == 13)
    {
        HAL_GPIO_WritePin(GPIOF, GPIO_PIN_6, GPIO_PIN_SET);         // 1
        HAL_GPIO_WritePin(GPIOF, GPIO_PIN_5, GPIO_PIN_SET);         // 1
    }
    else{
        log_err("3D sync delay mode out of range: %d\n", mode);
        return;
    }
}


void init_delay3D(void)
{
    // Read current 3D delay value
    float t_ms = rt_param.sync_3d_delay_arg;
    // Set 3D delay
    set_delay3D(t_ms);
}


void set_delay3D(float t_ms) {
    // 0. Hardware version check
    if (!is_3d_delay_supported()) {
        log_err("3D delay function not supported on current hardware version\n");
        return;
    }
    
    // 1. Input validation (t_ms in [-8.1, 8.1]) ( refer to final test result)
    if (t_ms < -8.1f || t_ms > 8.1f) {
        log_err("3D sync delay value out of range: %f\n", t_ms);
        return;
    }

    // 2. Process delay value and POL
    int32_t t_delay_us;                                 // Delay time (microseconds)
    t_delay_us = (int32_t)(t_ms * 1000);                // Convert to us
    uint32_t abs_t_us = (uint32_t)abs(t_delay_us);      // Absolute value

    uint8_t pol;                                        // POL flag (0 or 1)
    pol = (t_delay_us > 0) ? 0 : 1;                    // Sign determines POL: positive POL=0, negative POL=1
    
    if (pol == 1) abs_t_us = 8100 - abs_t_us;         // POL=1, abs_t_us = 8100 - abs_t_us
    if (abs_t_us < 64) abs_t_us = 64;                 // |t_ms| < 0.064ms (64us)
    if (abs_t_us > 8100) abs_t_us = 8100;             // |t_ms| > 8.1ms ( refer to final test result)

    // 3. Select DIVCODE mode (based on t_delay_us and POL)
    uint8_t divcode_mode; 
    if (pol == 0) {
        if (abs_t_us <= 512) {         // t_delay <= 0.512ms
            divcode_mode = 2;          // DIVCODE 2: POL=0, NDIV=64 (delay rising edge)
        } else {
            divcode_mode = 3;          // DIVCODE 3: POL=0, NDIV=512 (delay rising edge) 
        }
    } else { // pol == 1
        if (abs_t_us <= 512) {
            divcode_mode = 13;          // DIVCODE 13: POL=1, NDIV=64 (delay falling edge or invert)
        } else {
            divcode_mode = 12;          // DIVCODE 12: POL=1, NDIV=512 (delay falling edge or invert)
        }
    }

    // 4. Call GPIO selection function to set DIVCODE
    select_LTC6994_divcode_mode(divcode_mode);

    // 5. Calculate NDIV (based on DIVCODE selection)
    uint32_t ndiv;
    switch (divcode_mode) {
        case 2: // DIVCODE 2
        case 13: // DIVCODE 13
            ndiv = 64;     // tDELAY range: 64us to 1.024ms
            break;
        case 3: // DIVCODE 3
        case 12: // DIVCODE 12
            ndiv = 512;    // tDELAY range: 512us to 8.192ms
            break;
        default:
            ndiv = 64;     // Default safe value
            break;
    }

    // 6. Calculate RSET (unit: Ω) - Formula from LTC6994 datasheet: tDELAY_us = (NDIV * RSET) / 50000
    uint32_t rset = (abs_t_us * 50000) / ndiv;
    
    // RSET boundary constraints (50kΩ to 800kΩ)
    if (rset < 50000) rset = 50000;    // Minimum 50kΩ
    if (rset > 800000) rset = 800000;  // Maximum 800kΩ

    // 7. Calculate AD5241 Wiper position (assuming 1MΩ version: RAB=1MΩ, Wiper position D=0-255)
    // Formula: RWB ≈ (D/256) * RAB => D = (RSET / 1000000) * 256
    uint8_t d_position = (uint8_t)(( (rset - 50000) / 1000000.0) * 256);
    if (d_position > 255) d_position = 255; // Ensure within 0-255 range  

    // 8. Send I2C command to configure AD5241
    AD5241_write_value(d_position); // Set Wiper position

    log_info("3D sync delay set to %.3f ms, abs_t_us: %d, POL: %d, DIVCODE: %d, NDIV: %d, RSET: %d, D_POSITION: %d", 
             t_ms, abs_t_us, pol, divcode_mode, ndiv, rset, d_position);

    // 9. Save 3D delay value to configuration file
    rt_param.sync_3d_delay_arg = t_ms;
    save_rt_param_to_flash();

}

void get_delay3D(float *t_ms)
{
    // Hardware version check
    if (!is_3d_delay_supported()) {
        log_err("3D delay function not supported on current hardware version\n");
        *t_ms = 0.0f; // Return default value
        return;
    }
    
    *t_ms = rt_param.sync_3d_delay_arg;
}

/**
 * @brief Check if 3D delay function is supported by current hardware version
 * @return true if supported, false otherwise
 */
bool is_3d_delay_supported(void)
{
    uint8_t hw_version = read_hw_version();
    bool supported = (hw_version >= MIN_HW_VERSION_FOR_3D_DELAY);
    
    if (!supported) {
        log_warn("3D delay function not supported on hardware version %d (minimum required: %d)", 
                 hw_version, MIN_HW_VERSION_FOR_3D_DELAY);
    }
    
    return supported;
}

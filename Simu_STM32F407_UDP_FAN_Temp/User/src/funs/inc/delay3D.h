#ifndef __DELAY3D_H__
#define __DELAY3D_H__

#include "main.h"
#include <stdbool.h>

/**
 * @brief Initialize 3D delay
 * @note Initialize 3D delay according to configuration file
 */
void init_delay3D(void);

/**
 * @brief Set 3D delay
 * @note Core function to configure 3D sync delay IC LTC6994 and AD5241
 * @param t_ms Delay time in milliseconds
 */
void set_delay3D(float t_ms);

/**
 * @brief Get 3D delay
 * @param t_ms Delay time in milliseconds
 */
void get_delay3D(float *t_ms);

/**
 * @brief Check if 3D delay function is supported by current hardware version
 * @return true if supported, false otherwise
 */
bool is_3d_delay_supported(void);

#endif

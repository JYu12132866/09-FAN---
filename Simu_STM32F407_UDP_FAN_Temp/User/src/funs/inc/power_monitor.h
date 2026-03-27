#ifndef POWER_MONITOR_H_
#define POWER_MONITOR_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 每 1 秒周期调用，用于处理关机延迟计时与计数写入。
 */
void power_monitor_tick(void);

/**
 * @brief 记录一次开机事件。
 */
void power_monitor_on_power_on(void);

/**
 * @brief 记录一次关机事件，启动 30 秒延迟确认。
 */
void power_monitor_on_power_off(void);

/**
 * @brief 清零开关机计数，并取消待确认关机状态。
 */
void power_monitor_reset_counters(void);

/**
 * @brief 读取累计开机次数。
 */
int power_monitor_get_power_on_count(void);

/**
 * @brief 读取累计关机次数。
 */
int power_monitor_get_power_off_count(void);

#ifdef __cplusplus
}
#endif

#endif /* POWER_MONITOR_H_ */


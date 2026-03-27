#include "power_monitor.h"
#include <stdio.h>
#include "main.h"
#include "logger.h"
#include "rt_param.h"
#include <stdbool.h>

#define POWER_MONITOR_SHUTDOWN_DELAY_SECONDS (30U)

static bool s_pending = 0;
static uint16_t s_pending_seconds = 0;

static void power_monitor_increment   (int *counter);

void power_monitor_tick(void)
{
    if (s_pending == false)
    {
        return;
    }

    if (s_pending_seconds > 0U)
    {
        s_pending_seconds--;
    }

    if (s_pending_seconds == 0U)
    {
        power_monitor_increment(&rt_param.power_off_count);
        s_pending = false;
        save_rt_param_to_flash();
        log_info("Power monitor: normal shutdown recorded.");
    }
}

void power_monitor_on_power_on(void)
{
    if (s_pending)
    {
        s_pending = 0;
        s_pending_seconds = 0;
        log_info("Power monitor: pending shutdown cancelled by power on.");
    }

    power_monitor_increment(&rt_param.power_on_count);
    save_rt_param_to_flash();
    log_info("Power monitor: power on count = %lu.", (unsigned long)rt_param.power_on_count);
}

void power_monitor_on_power_off(void)
{
    s_pending = 1;
    s_pending_seconds = POWER_MONITOR_SHUTDOWN_DELAY_SECONDS;
    log_info("Power monitor: shutdown pending, will confirm in %u seconds.", (unsigned int)POWER_MONITOR_SHUTDOWN_DELAY_SECONDS);
}

void power_monitor_reset_counters(void)
{
    s_pending = 0;
    s_pending_seconds = 0;

    rt_param.power_on_count = 0;
    rt_param.power_off_count = 0;

    save_rt_param_to_flash();
    log_info("Power monitor: counters reset.");
}

int power_monitor_get_power_on_count(void)
{
    return rt_param.power_on_count;
}

int power_monitor_get_power_off_count(void)
{
    return rt_param.power_off_count;
}


static void power_monitor_increment(int *counter)
{
    if (counter == NULL)
    {
        return;
    }

    if (*counter != 0xFFFFFFFF)
    {
        (*counter)++;
    }
}


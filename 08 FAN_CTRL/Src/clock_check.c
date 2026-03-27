/**
 * @file    clock_check.c
 * @brief   时钟检测工具文件
 * @note    用于读取和显示 GD32/STM32 的实际时钟频率
 */

#include "main.h"

/**
 * @brief 读取并显示系统时钟频率
 */
void ReadAndDisplayClockFrequencies(void)
{
    // 读取各总线时钟频率
    uint32_t hclk_freq = HAL_RCC_GetHCLKFreq();
    uint32_t pclk1_freq = HAL_RCC_GetPCLK1Freq();
    uint32_t pclk2_freq = HAL_RCC_GetPCLK2Freq();
    uint32_t sysclk_freq = HAL_RCC_GetSysClockFreq();

    // 这里可以添加代码将时钟频率输出到串口
    // 例如使用 printf 输出到 UART1
    // printf("SYSCLK: %lu MHz\r\n", sysclk_freq / 1000000);
    // printf("HCLK:   %lu MHz\r\n", hclk_freq / 1000000);
    // printf("PCLK1:  %lu MHz\r\n", pclk1_freq / 1000000);
    // printf("PCLK2:  %lu MHz\r\n", pclk2_freq / 1000000);

    // 简单的时钟验证方法：
    // 1. 使用示波器测量 MCO 引脚（如果已配置）
    // 2. 使用串口输出时钟频率（需要初始化 UART）
    // 3. 使用 ST-Link Utility 或 GD-Link 工具读取时钟
    // 4. 测量定时器输出频率反推时钟

    // 如果时钟频率异常，检查 SystemClock_Config() 中的配置
    // - PLL 倍频系数是否正确
    // - HSE 是否工作（检查晶振）
    // - 分频系数是否正确
}

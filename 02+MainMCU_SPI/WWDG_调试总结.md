# 窗口看门狗(WWDG)配置与调试总结

## 项目背景
- **MCU**: GD32F303RCT6 (兼容STM32F1xx)
- **开发环境**: STM32CubeMX + Keil MDK5 + STM32 HAL库
- **功能需求**: LED1和LED2交替闪烁，间隔500ms，同时配置看门狗监控系统运行

## 问题现象
1. LED1常亮，LED2不亮
2. LED无法交替闪烁
3. 添加看门狗后现象更严重

## 根本原因分析

### WWDG超时时间计算错误

**时钟配置**:
- SYSCLK: 72MHz (HSE 8MHz × PLL9)
- PCLK1: 36MHz (SYSCLK ÷ 2)
- WWDG时钟: PCLK1 ÷ 8 = 4.5MHz

**WWDG参数**:
- Prescaler: 8 (最大值)
- Counter: 0x7F (127)
- Window: 0x20 (32)

**超时时间计算**:
```
WWDG超时时间 = Counter × Prescaler / PCLK1
             = 127 × 8 / 36,000,000
             = 28.22 μs
```

**关键发现**: WWDG最大超时时间仅28微秒！

### 问题复现流程
1. 调用 `MX_WWDG_Init()` 启动看门狗
2. LED1置为SET，LED2置为RESET
3. 执行 `HAL_Delay(500)`
4. **28μs后WWDG超时触发系统复位**
5. 重复步骤2-4，LED1永远保持SET状态

## WWDG工作原理

### 窗口看门狗特点
1. **窗口机制**: 必须在计数器值降到Window值以下后才能喂狗
   - 如果在计数器 > Window值时喂狗 → 触发复位
   - 如果计数器降到0x3F以下 → 触发复位

2. **喂狗时间窗口**:
   ```
   喂狗窗口 = (Counter - Window) × Prescaler / PCLK1
   有效范围: 21μs ~ 28μs
   ```

3. **适用场景**: 监测高频实时任务，不适合与HAL_Delay等长延时函数配合使用

### WWDG参数限制 (STM32F1xx)
```c
// 预分频选项有限
WWDG_PRESCALER_1
WWDG_PRESCALER_2
WWDG_PRESCALER_4
WWDG_PRESCALER_8  // 最大值
```

## 解决方案

### 方案1: 禁用WWDG (当前方案)
```c
// main.c
// MX_WWDG_Init();  // 暂时禁用WWDG
```

**优点**: 简单直接，立即解决问题
**缺点**: 缺少系统监控机制

### 方案2: 使用独立看门狗(IWDG)
**优点**:
- 超时时间可达秒级（使用LSI 40KHz）
- 无窗口限制，任意时刻可喂狗
- 适合长延时场景

**配置示例**:
```c
hiwdg.Instance = IWDG;
hiwdg.Init.Prescaler = IWDG_PRESCALER_64;  // 40KHz/64 = 625Hz
hiwdg.Init.Reload = 0x0FFF;                // 4095
// 超时时间 = 65 * 4095 / 40000 ≈ 6.65秒
```

### 方案3: 软件看门狗
使用定时器实现，超时时间可精确控制

## GPIO相关问题

### JTAG引脚冲突
**问题**: PA15是JTAG的JTMS引脚，默认不能作为GPIO使用

**解决**:
```c
__HAL_RCC_AFIO_CLK_ENABLE();
__HAL_AFIO_REMAP_SWJ_NOJTAG();  // 禁用JTAG，保留SWD
```

### LED引脚定义
- LED1: PA15 (GPIO_PIN_15)
- LED2: PC5 (GPIO_PIN_5)

## 调试经验总结

### 1. 现象分析
| 现象 | 可能原因 | 排查方法 |
|------|---------|---------|
| LED1常亮LED2不亮 | 看门狗复位、GPIO配置错误 | 检查WWDG调用、验证引脚初始化 |
| LED常亮不闪烁 | HAL_Delay失效、看门狗复位 | 检查SysTick、确认WWDG状态 |
| 编译错误 | HAL模块未启用、驱动文件缺失 | 检查hal_conf.h宏定义 |

### 2. 关键调试点
1. **看门狗初始化时机**: 确认WWDG是否被意外调用
2. **超时时间计算**: 精确计算WWDG超时，避免与延时冲突
3. **GPIO复用功能**: 检查JTAG/SWD是否禁用
4. **时钟配置**: 验证PCLK1频率，影响WWDG超时

### 3. 代码审查清单
- [ ] WWDG初始化是否必要？
- [ ] WWDG超时时间是否与应用延时匹配？
- [ ] 是否有多次调用 `MX_WWDG_Init()`？
- [ ] GPIO复用功能是否正确配置？
- [ ] 是否使用了正确的预分频选项？

## 最终代码

### main.c
```c
int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();

  /* 禁用JTAG释放PA15 */
  __HAL_RCC_AFIO_CLK_ENABLE();
  __HAL_AFIO_REMAP_SWJ_NOJTAG();

  /* WWDG已禁用 - 超时时间(28μs)与HAL_Delay(500ms)不匹配 */
  // MX_WWDG_Init();

  while (1)
  {
    /* LED1亮，LED2灭 */
    HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET);
    HAL_Delay(500);

    /* LED1灭，LED2亮 */
    HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET);
    HAL_Delay(500);
  }
}
```

## 解决方案（最终成功方案）

### 使用WWDG实现LED闪烁
**核心思路**: 通过软件延时配合WWDG喂狗，实现长时间延时（500ms）

#### 关键配置调整
1. **禁用窗口机制**: `Window = 0x7F`（与Counter相同）
   - 原配置 `Window = 0x20` 导致只能在21μs~28μs窗口内喂狗
   - 改为 `Window = 0x7F` 后，只要在28μs内喂狗即可
   - 避免了"过早喂狗触发复位"的问题

2. **软件延时实现500ms**:
   - WWDG超时时间：28μs
   - 500ms / 28μs ≈ 20000次喂狗
   - 每次喂狗间隔约25μs（通过软件循环实现）

#### wwddg.c 配置
```c
hwwdg.Instance = WWDG;
hwwdg.Init.Prescaler = WWDG_PRESCALER_8;   // 8分频（最大）
hwwdg.Init.Window = 0x7F;                 // 窗口值=0x7F，禁用窗口机制
hwwdg.Init.Counter = 0x7F;                 // 计数器值：最大超时时间
hwwdg.Init.EWIMode = WWDG_EWI_DISABLE;  // 禁用早期唤醒中断
```

#### main.c 实现
```c
/* Private variables */
static volatile uint32_t wwdg_counter = 0;
static GPIO_PinState led1_state = GPIO_PIN_RESET;
static GPIO_PinState led2_state = GPIO_PIN_SET;

int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();

  /* Disable JTAG to release PA15 (JTMS) as GPIO */
  __HAL_RCC_AFIO_CLK_ENABLE();
  __HAL_AFIO_REMAP_SWJ_NOJTAG();

  /* 启用窗口看门狗 */
  MX_WWDG_Init();

  /* 初始化LED状态 */
  led1_state = GPIO_PIN_SET;
  led2_state = GPIO_PIN_RESET;
  HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, led1_state);
  HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, led2_state);

  while (1)
  {
    /* 使用计数器实现500ms延时，同时保持喂狗 */
    /* WWDG需要每约25μs喂狗一次 */
    /* 500ms / 25μs = 20000次喂狗 */

    uint32_t feed_count = 0;
    const uint32_t FEED_TARGET = 20000;

    while (feed_count < FEED_TARGET) {
      /* 短暂延时（约25μs） */
      volatile uint32_t delay = 100;
      while (delay--);

      /* 喂狗 */
      HAL_WWDG_Refresh(&hwwdg);
      feed_count++;
    }

    /* 切换LED状态 */
    led1_state = (led1_state == GPIO_PIN_SET) ? GPIO_PIN_RESET : GPIO_PIN_SET;
    led2_state = (led2_state == GPIO_PIN_SET) ? GPIO_PIN_RESET : GPIO_PIN_SET;
    HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, led1_state);
    HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, led2_state);
  }
}
```

### 方案对比

| 方案 | 超时时间 | 优点 | 缺点 | 适用场景 |
|------|----------|------|------|---------|
| **WWDG (成功方案)** | 最大28μs | 监控粒度细、硬件实现 | 需频繁喂狗 | 高频实时任务 |
| IWDG | 最大26秒 | 长超时、易使用 | 粒度较粗 | 通用应用监控 |
| 软件看门狗 | 可配置 | 灵活可调 | 依赖系统时钟 | 特定需求 |

## 结论

1. **WWDG可以用于长延时场景**: 通过禁用窗口机制（Window=0x7F）和软件延时，可以实现500ms级延时
2. **关键配置**: Window=0x7F禁用窗口限制，允许任意时刻喂狗
3. **喂狗策略**: 每约25μs喂狗一次，累计20000次实现500ms延时
4. **性能权衡**: 虽然WWDG可以工作，但频繁喂狗会占用CPU时间，实际应用中推荐IWDG

## 参考资料
- STM32F1xx HAL库用户手册
- GD32F303数据手册
- STM32窗口看门狗应用笔记AN4793

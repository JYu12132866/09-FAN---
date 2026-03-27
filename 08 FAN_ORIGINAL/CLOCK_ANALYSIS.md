# GD32F303RCT6 时钟配置与验证说明

## 一、当前时钟配置

### 1.1 配置参数

| 时钟源 | 频率 | 说明 |
|--------|--------|------|
| HSE (外部晶振) | 8 MHz | 高频外部晶振 |
| PLL 倍频系数 | ×9 | 8MHz × 9 = 72MHz |
| SYSCLK (系统时钟) | 72 MHz | 系统核心时钟 |
| HCLK (AHB总线) | 72 MHz | 不分频 |
| PCLK1 (APB1总线) | 36 MHz | HCLK/2，看门狗时钟源 |
| PCLK2 (APB2总线) | 72 MHz | HCLK/1，高速外设时钟 |
| Flash 延迟 | FLASH_LATENCY_2 | 适用于 48-72MHz |

### 1.2 配置合理性验证

✓ **GD32F303RCT6 最高主频 120MHz，72MHz 在安全范围内**
✓ **PLL ×9 倍频在 GD32F303 支持范围内**
✓ **Flash 延迟配置正确（72MHz 需要 2 个等待周期）**
✓ **各分频系数合理，外设时钟在正常范围内**

---

## 二、如何判断烧录后的实际时钟频率

### 方法 1：使用 HAL 库函数读取时钟（推荐）

在 `main.c` 的 `SystemClock_Config()` 之后添加：

```c
/* 获取实际时钟频率 */
uint32_t sysclk_freq = HAL_RCC_GetSysClockFreq();
uint32_t hclk_freq = HAL_RCC_GetHCLKFreq();
uint32_t pclk1_freq = HAL_RCC_GetPCLK1Freq();
uint32_t pclk2_freq = HAL_RCC_GetPCLK2Freq();

// 可以通过串口输出（需要初始化 UART）
// printf("SYSCLK: %lu MHz\r\n", sysclk_freq / 1000000);
// printf("HCLK:   %lu MHz\r\n", hclk_freq / 1000000);
// printf("PCLK1:  %lu MHz\r\n", pclk1_freq / 1000000);
// printf("PCLK2:  %lu MHz\r\n", pclk2_freq / 1000000);

// 验证时钟是否正确
// 如果 sysclk_freq = 72000000，说明时钟配置正确
// 如果 sysclk_freq != 72000000，说明时钟配置有问题
```

**如何使用：**
1. 添加上述代码到 main 函数
2. 设置断点在 `HAL_RCC_GetSysClockFreq()` 之后
3. 使用调试器查看 `sysclk_freq` 变量的值
4. 对比值是否为 72000000（72MHz）

### 方法 2：测量 MCO 引脚输出

1. 在 STM32CubeMX 中配置 MCO 引脚输出时钟
2. 使用示波器或频率计测量 MCO 引脚
3. 对比测量值与配置值

### 方法 3：测量定时器输出

1. 配置一个定时器输出 PWM（如 TIM1_CH1）
2. 测量输出频率
3. 反推系统时钟：
   ```
   SysClk = 测量频率 × 分频系数
   ```

### 方法 4：测量 HAL_Delay 的准确性

```c
// 简单测试代码
while (1) {
    HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
    HAL_Delay(1000);  // 理论上应该是 1 秒
}
```

**验证步骤：**
1. 使用秒表测量 LED 闪烁周期
2. 如果闪烁周期是 1 秒，说明 SysTick 正常
3. 如果闪烁周期不是 1 秒，说明系统时钟有偏差

---

## 三、ST-Link "Not a genuine ST Device!" 报错说明

### 3.1 报错原因

- ST-Link 烧录器检测到芯片 ID 不是 ST 系列
- GD32 芯片 ID 为 `0x430` 系列（如 0x43003010）
- ST 芯片 ID 为 `0x410/411/412` 系列

### 3.2 是否影响

| 项目 | 影响 |
|------|--------|
| 程序烧录 | ✓ 不影响，可以正常烧录 |
| 程序运行 | ✓ 不影响，代码完全兼容 |
| 时钟配置 | ✓ 不影响，时钟配置代码通用 |
| 功能性能 | ✓ 不影响，性能一致 |

### 3.3 如何解决

**方法 1：忽略警告（推荐）**
- 在 STM32CubeIDE 中设置忽略此警告
- 或使用其他烧录器（J-Link、GD-Link）

**方法 2：使用 GD 专用工具**
- GD32F303 官方开发套件
- Keil MDK for GD32

**方法 3：修改烧录器配置**
- 某些烧录器可以禁用芯片检测
- 查看 ST-Link Utility 的配置选项

---

## 四、当前配置验证方法

### 4.1 LED 闪烁周期验证

- 代码中 `HAL_Delay(1000)` 理论上应该是 1 秒
- 用秒表测量 LED 闪烁周期
- **如果闪烁周期正确（2 秒一个循环），说明时钟正常**

### 4.2 建议添加的验证代码

在 main 函数中 `SystemClock_Config()` 之后添加：

```c
/* 时钟验证代码 */
uint32_t sysclk_freq = HAL_RCC_GetSysClockFreq();
uint32_t hclk_freq = HAL_RCC_GetHCLKFreq();
uint32_t pclk1_freq = HAL_RCC_GetPCLK1Freq();

// 可以设置断点查看这些变量的值
// sysclk_freq 应该是 72000000（72MHz）
// hclk_freq 应该是 72000000（72MHz）
// pclk1_freq 应该是 36000000（36MHz）
```

**如何使用：**
1. 添加上述代码
2. 在调试模式下运行程序
3. 设置断点在 `HAL_RCC_GetSysClockFreq()` 之后
4. 查看变量值是否正确

### 4.3 看门狗时钟验证

- 看门狗使用 PCLK1 = 36MHz
- 超时时间计算（使用当前配置）：
  ```
  T_max = (4096 × 8) / 36000000 × 127 ≈ 145ms
  ```
- 如果喂狗间隔 > 145ms，系统会复位
- 当前禁用看门狗，可以先验证基本功能

---

## 五、GD32F303 与 STM32F1xx 的时钟差异

### 5.1 内部 RC 振荡器精度

| 芯片 | HSI 精度 | 说明 |
|------|-----------|------|
| STM32 | ±1% | 约 8MHz |
| GD32 | 可能有细微差异 | 建议使用 HSE |

**当前配置使用 HSE（外部晶振），精度更高，建议保持。**

### 5.2 PLL 稳定性

- GD32 的 PLL 可能在高频时有细微差异
- 72MHz 在稳定范围内，问题不大
- 如果使用 120MHz，需要测试稳定性

### 5.3 时钟树结构

- GD32F303 与 STM32F1xx 基本兼容
- 某些寄存器定义可能有细微差异
- **HAL 库已经做了适配，可以直接使用**

---

## 六、时钟配置总结

| 项目 | 配置值 | 验证结果 |
|------|---------|---------|
| 外部晶振 | 8 MHz HSE | ✓ 合理 |
| PLL 倍频 | ×9 | ✓ 合理 |
| SYSCLK | 72 MHz | ✓ 在 GD32F303 范围内 |
| HCLK | 72 MHz | ✓ 不分频，正常 |
| PCLK1 | 36 MHz | ✓ HCLK/2，看门狗时钟源 |
| PCLK2 | 72 MHz | ✓ HCLK/1，高速外设时钟 |
| Flash 延迟 | FLASH_LATENCY_2 | ✓ 适用于 72MHz |

**配置公式：**
```
SYSCLK = HSE × PLLMUL = 8MHz × 9 = 72MHz
HCLK   = SYSCLK / 1 = 72MHz / 1 = 72MHz
PCLK1  = HCLK / 2 = 72MHz / 2 = 36MHz
PCLK2  = HCLK / 1 = 72MHz / 1 = 72MHz
```

**当前配置的验证结果：**
- LED 可以正常交替闪烁 ✓
- `HAL_Delay(1000)` 实际测量接近 1 秒 ✓
- 说明时钟配置正确，系统正常工作 ✓

---

## 七、如需使用 120MHz（GD32F303 最高性能）

### 7.1 修改时钟配置

```c
// 在 SystemClock_Config() 中修改
RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL15;  // 8MHz × 15 = 120MHz
RCC_ClkInitStruct.FlashLatency = FLASH_LATENCY_3;  // 120MHz 需要 3 个等待周期
```

### 7.2 120MHz 时的时钟

| 时钟 | 频率 |
|------|--------|
| SYSCLK | 120 MHz |
| HCLK | 120 MHz |
| PCLK1 | 60 MHz |
| PCLK2 | 120 MHz |

### 7.3 看门狗超时计算（120MHz）

```
PCLK1 = 60MHz
T_max = (4096 × 8) / 60000000 × 127 ≈ 87ms
```

**喂狗间隔必须小于 87ms**

### 7.4 注意事项

- 需要测试系统稳定性
- 可能需要增加电源去耦电容
- 某些外设在 120MHz 时可能有时序问题
- 建议逐步测试：72MHz → 96MHz → 120MHz

---

## 八、常见问题与解决方案

### Q1: LED 闪烁周期不对

**原因：**
- 时钟配置错误
- HSE 晶振不工作

**解决：**
1. 检查晶振是否焊接正确
2. 使用 `HAL_RCC_GetSysClockFreq()` 读取实际时钟
3. 尝试使用 HSI（内部振荡器）

### Q2: 系统不断复位

**原因：**
- 看门狗超时
- 时钟不稳定

**解决：**
1. 禁用看门狗验证
2. 增大看门狗超时时间（增大 Prescaler）
3. 检查时钟配置

### Q3: HAL_Delay 不准

**原因：**
- SysTick 时钟错误
- 系统时钟偏差

**解决：**
1. 测量 LED 闪烁周期
2. 使用示波器测量时钟
3. 检查 SystemCoreClockUpdate()

---

## 九、参考文档

1. **GD32F30x 用户手册**（时钟部分）
2. **STM32F1xx 参考手册**（兼容性说明）
3. **STM32 HAL 库文档**（时钟配置函数）
4. **GD32 官方技术支持**

---

## 十、当前配置总结

**时钟配置：** 8MHz HSE → PLL ×9 → 72MHz SYSCLK
**验证状态：** ✓ LED 正常交替闪烁，时钟工作正常
**ST-Link 警告：** ✓ 可以忽略，不影响使用
**看门狗状态：** 已禁用（先确保基本功能正常）

**结论：当前时钟配置合理，系统正常工作。**

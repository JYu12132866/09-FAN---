# STM32 TEC控制器通信系统 - 新手入门指南

## 📋 目录
1. [项目概述](#项目概述)
2. [硬件平台](#硬件平台)
3. [开发环境](#开发环境)
4. [项目结构](#项目结构)
5. [模块功能详解](#模块功能详解)
6. [代码阅读指南](#代码阅读指南)
7. [调试技巧](#调试技巧)
8. [常见问题](#常见问题)

---

## 项目概述

这是一个基于STM32F103RE微控制器的TEC（热电冷却器）控制器通信系统，主要功能包括：

- **串口通信**：通过USART1（115200bps）与PC串口助手通信，通过USART2（9600bps）与TEC控制器通信
- **PWM控制**：输出4路PWM波形（100Hz）控制风扇和水泵
- **信号采集**：通过I2C接口读取NTC温度传感器数据
- **信号检测**：通过TIM2输入捕获检测风扇FG（转速反馈）信号
- **数据转发**：将PC串口助手的指令转发给TEC控制器，并将TEC控制器的响应回传给PC

### 系统架构图
```
+------------------+     +------------------+     +------------------+
|   PC 串口助手    |     |   STM32 MCU      |     |   TEC 控制器     |
| (115200 bps)     |<--->| (USART1)         |<--->| (9600 bps)       |
+------------------+     +------------------+     +------------------+
                                |
                                | I2C (PB10/SCL, PB11/SDA)
                                v
                        +------------------+
                        |   TLA2528 ADC    |
                        |   (NTC 温度)     |
                        +------------------+
                                |
                                | TIM2_CH2 (PA1)
                                v
                        +------------------+
                        |   FG 信号输入    |
                        |   (风扇转速反馈)  |
                        +------------------+
```

---

## 硬件平台

### 主控芯片
- **型号**：STM32F103RE
- **内核**：Cortex-M3
- **主频**：72MHz（通过PLL倍频）
- **Flash**：512KB
- **RAM**：64KB

### 关键外设
| 外设 | 引脚 | 功能 |
|------|------|------|
| USART1_TX | PA9 | 串口助手通信（115200bps） |
| USART1_RX | PA10 | 串口助手通信（115200bps） |
| USART2_TX | PA2 | TEC控制器通信（9600bps） |
| USART2_RX | PA3 | TEC控制器通信（9600bps） |
| I2C2_SCL | PB10 | TLA2528 ADC时钟线 |
| I2C2_SDA | PB11 | TLA2528 ADC数据线 |
| TIM3_CH1~CH4 | PC6~PC9 | PWM输出（PUMP/FAN1/FAN2/FAN3） |
| TIM2_CH2 | PA1 | FG信号输入（风扇转速反馈） |

### 电源和时钟
- **系统时钟源**：HSI（内部8MHz RC振荡器）+ PLL倍频至72MHz
- **APB1时钟**：72MHz / 2 = 36MHz
- **APB2时钟**：72MHz（不分频）

---

## 开发环境

### 软件工具
1. **Keil MDK-ARM**：集成开发环境（IDE）
2. **STM32CubeMX**：图形化配置工具（生成初始化代码）
3. **串口助手**：友善

### 编译流程
1. 打开Keil MDK-ARM
2. 加载项目：`MDK-ARM\EdgeMCU_test.uvprojx`
3. 点击"Build"按钮（或按F7）
4. 编译成功后，固件位于：`MDK-ARM\EdgeMCU_test\EdgeMCU_test.hex`

### 下载流程
1. 连接ST-Link调试器到开发板
2. 在Keil中点击"Download"按钮（或按Ctrl+D）
3. 等待下载完成

---

## 项目结构

```
06 RS232/
├── Drivers/                    # STM32 HAL驱动库（ST官方提供）
│   ├── CMSIS/                 # ARM Cortex-M核心库
│   └── STM32F1xx_HAL_Driver/  # STM32F1系列HAL驱动
├── Inc/                       # 头文件（.h）
│   ├── main.h                 # 主程序头文件
│   ├── gpio.h                 # GPIO驱动头文件
│   ├── tim.h                  # TIM驱动头文件
│   ├── usart.h                # USART驱动头文件
│   ├── i2c.h                  # I2C驱动头文件
│   ├── spi.h                  # SPI驱动头文件
│   └── stm32f1xx_it.h         # 中断处理头文件
├── Src/                       # 源文件（.c）
│   ├── main.c                 # 主程序
│   ├── gpio.c                 # GPIO驱动实现
│   ├── tim.c                  # TIM驱动实现
│   ├── usart.c                # USART驱动实现
│   ├── i2c.c                  # I2C驱动实现
│   ├── spi.c                  # SPI驱动实现
│   ├── stm32f1xx_it.c         # 中断服务程序
│   └── system_stm32f1xx.c     # 系统时钟配置
├── MDK-ARM/                   # Keil项目文件
│   ├── EdgeMCU_test.uvprojx   # Keil项目文件
│   └── EdgeMCU_test.hex       # 编译后的固件
└── README.md                  # 本文件
```

---

## 模块功能详解

### 1. GPIO模块（通用输入输出）

#### 文件位置
- 头文件：`Inc/gpio.h`
- 源文件：`Src/gpio.c`

#### 功能描述
GPIO模块负责配置所有使用的引脚，包括：
- **LED指示灯**：LED1（PA15）、LED2（PC5）
- **MUX控制**：MUX_EN0（PA11）、MUX_EN1（PA12）、MUX_ADDR0-2（PB3-5）
- **串口引脚**：USART1_TX（PA9）、USART1_RX（PA10）、USART2_TX（PA2）、USART2_RX（PA3）
- **I2C引脚**：I2C2_SCL（PB10）、I2C2_SDA（PB11）
- **PWM引脚**：TIM3_CH1~CH4（PC6-PC9）

#### 关键代码解析

```c
// gpio.c - MX_GPIO_Init() 函数

void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};  // 定义GPIO配置结构体

  // 1. 使能GPIO端口时钟
  __HAL_RCC_GPIOD_CLK_ENABLE();  // 使能GPIOD时钟
  __HAL_RCC_GPIOA_CLK_ENABLE();  // 使能GPIOA时钟
  __HAL_RCC_GPIOC_CLK_ENABLE();  // 使能GPIOC时钟
  __HAL_RCC_GPIOB_CLK_ENABLE();  // 使能GPIOB时钟

  // 2. 配置LED2（PC5）为推挽输出
  GPIO_InitStruct.Pin = LED2_Pin;        // 选择引脚
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;  // 推挽输出模式
  GPIO_InitStruct.Pull = GPIO_NOPULL;    // 无上下拉
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW; // 低速
  HAL_GPIO_Init(LED2_GPIO_Port, &GPIO_InitStruct);  // 初始化

  // 3. 配置MUX控制引脚（PA11, PA12, PB3~5）
  GPIO_InitStruct.Pin = MUX_EN0_Pin|MUX_EN1_Pin;  // 多个引脚
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);  // 初始化GPIOA
}
```

#### 代码逐行讲解

| 代码行 | 说明 |
|--------|------|
| `GPIO_InitTypeDef GPIO_InitStruct = {0};` | 定义一个GPIO配置结构体，并初始化为0 |
| `__HAL_RCC_GPIOD_CLK_ENABLE();` | 宏函数，使能GPIOD端口的时钟。所有GPIO操作前必须先使能时钟 |
| `GPIO_InitStruct.Pin = LED2_Pin;` | 设置要配置的引脚为LED2（PC5） |
| `GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;` | 设置模式为推挽输出（可以主动输出高/低电平） |
| `GPIO_InitStruct.Pull = GPIO_NOPULL;` | 设置无上下拉电阻 |
| `HAL_GPIO_Init(LED2_GPIO_Port, &GPIO_InitStruct);` | 调用HAL库函数，使用配置结构体初始化GPIO |

#### 常用GPIO操作

```c
// 设置引脚为高电平
HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);

// 设置引脚为低电平
HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);

// 读取引脚电平
GPIO_PinState state = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_5);
if (state == GPIO_PIN_SET) {
    // 引脚为高电平
}
```

---

### 2. USART模块（串口通信）

#### 文件位置
- 头文件：`Inc/usart.h`
- 源文件：`Src/usart.c`

#### 功能描述
USART模块实现两个串口的配置：
- **USART1**：115200bps，用于与PC串口助手通信
- **USART2**：9600bps，用于与TEC控制器通信

#### 关键配置

```c
// usart.c - MX_USART1_UART_Init() 函数

void MX_USART1_UART_Init(void)
{
  huart1.Instance = USART1;  // 选择USART1外设
  huart1.Init.BaudRate = 115200;  // 波特率：115200
  huart1.Init.WordLength = UART_WORDLENGTH_8B;  // 数据位：8位
  huart1.Init.StopBits = UART_STOPBITS_1;  // 停止位：1位
  huart1.Init.Parity = UART_PARITY_NONE;  // 校验位：无
  huart1.Init.Mode = UART_MODE_TX_RX;  // 模式：收发模式
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;  // 硬件流控：禁用
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;  // 过采样：16倍
  
  if (HAL_UART_Init(&huart1) != HAL_OK)  // 初始化串口
  {
    Error_Handler();  // 初始化失败，进入错误处理
  }
}
```

#### 波特率计算原理

```
波特率 = f_CLK / (16 * USARTDIV)

其中：
- f_CLK = 36MHz (APB1时钟)
- USARTDIV = 36000000 / (16 * 115200) = 19.53125

HAL库会自动计算并设置USARTDIV寄存器
```

#### 中断接收机制

```c
// usart.c - USART1中断接收配置

// 1. 启动中断接收（在main.c中）
rx_index = 0;
HAL_UART_Receive_IT(&huart1, &rx_buffer[rx_index], 1);

// 2. 中断回调函数（在usart.c中）
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == USART1)
  {
    rx_buffer[rx_index] = rx_buffer[rx_index];  // 接收到的数据
    rx_index++;
    
    // 继续接收下一个字节
    HAL_UART_Receive_IT(&huart1, &rx_buffer[rx_index], 1);
  }
}
```

#### 数据接收流程

```
1. 主程序调用 HAL_UART_Receive_IT() 启动中断接收
2. 每收到1个字节，触发USART1_IRQHandler中断
3. HAL_UART_IRQHandler() 调用 HAL_UART_RxCpltCallback() 回调函数
4. 在回调函数中处理接收到的数据，并启动下一次接收
5. 主程序通过 USART1_GetReceivedData() 获取完整的命令字符串
```

#### 命令解析函数

```c
// main.c - trim_cmd() 函数：去除字符串首尾空白
static void trim_cmd(char *s)
{
    if (s == NULL) return;

    // 1. 去掉尾部空白（空格、制表符、换行、回车）
    size_t n = strlen(s);
    while (n > 0 && (s[n - 1] == ' ' || s[n - 1] == '\t' || 
                     s[n - 1] == '\n' || s[n - 1] == '\r')) {
        s[n - 1] = '\0';  // 用'\0'结束字符串
        n--;
    }

    // 2. 去掉头部空白
    size_t i = 0;
    while (s[i] == ' ' || s[i] == '\t' || s[i] == '\n' || s[i] == '\r') {
        i++;
    }
    
    // 3. 如果有头部空白，将字符串左移
    if (i > 0) {
        memmove(s, s + i, strlen(s + i) + 1);
    }
}
```

---

### 3. TIM模块（定时器/PWM）

#### 文件位置
- 头文件：`Inc/tim.h`
- 源文件：`Src/tim.c`

#### 功能描述
TIM模块实现以下功能：
- **TIM2**：输入捕获模式，用于捕获FG信号（风扇转速反馈）
- **TIM3**：PWM输出模式，输出4路PWM波形（100Hz）控制PUMP/FAN1/FAN2/FAN3
- **TIM4**：PWM输出模式，输出2路PWM波形控制LED
- **TIM8**：PWM输出模式（备用）

#### PWM工作原理

```
PWM频率 = 定时器时钟频率 / ((PSC + 1) * (ARR + 1))

其中：
- PSC（Prescaler）：预分频器
- ARR（Auto-Reload Register）：自动重载寄存器
- CCR（Capture/Compare Register）：比较寄存器（决定占空比）
```

#### TIM3 PWM配置

```c
// tim.c - MX_TIM3_Init() 函数

void MX_TIM3_Init(void)
{
  htim3.Instance = TIM3;  // 选择TIM3外设
  
  // 1. 计算PSC和ARR值（目标频率100Hz）
  uint32_t tim_clk = HAL_RCC_GetPCLK1Freq();  // 获取APB1时钟频率（36MHz）
  if ((RCC->CFGR & RCC_CFGR_PPRE1) != RCC_CFGR_PPRE1_DIV1) {
    tim_clk *= 2U;  // APB1分频不为1时，定时器时钟翻倍
  }
  
  const uint32_t pwm_hz = 100U;  // 目标频率：100Hz
  const uint32_t arr = 1000U - 1U;  // ARR = 999，分辨率1000级
  const uint32_t target_cnt_hz = pwm_hz * (arr + 1U);  // 100 * 1000 = 100kHz
  uint32_t psc = (tim_clk + (target_cnt_hz / 2U)) / target_cnt_hz;  // 四舍五入
  if (psc == 0U) psc = 1U;
  psc -= 1U;  // PSC = 359（实际值）
  
  htim3.Init.Prescaler = (uint16_t)psc;  // PSC = 359
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;  // 向上计数模式
  htim3.Init.Period = (uint16_t)arr;  // ARR = 999
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK) {
    Error_Handler();
  }
  
  // 2. 配置PWM通道（50%占空比）
  sConfigOC.OCMode = TIM_OCMODE_PWM1;  // PWM模式1
  sConfigOC.Pulse = (arr + 1U) / 2U;  // CCR = 500（50%占空比）
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  
  HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1);
  HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_2);
  HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_3);
  HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_4);
  
  // 3. 启动PWM输出
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_4);
}
```

#### PWM占空比计算

```
占空比 = (CCR / (ARR + 1)) * 100%

例如：
- ARR = 999
- CCR = 500
- 占空比 = (500 / 1000) * 100% = 50%
```

#### 修改占空比

```c
// main.c - FANPWM_SetDutyPercent() 函数

static void FANPWM_SetDutyPercent(uint8_t duty_percent)
{
  uint32_t arr = __HAL_TIM_GET_AUTORELOAD(&htim3);  // 获取ARR值
  uint32_t compare = ((arr + 1U) * duty_percent) / 100U;  // 计算CCR值
  if (compare > arr) compare = arr;  // 防止溢出
  
  // 设置4个通道的占空比
  __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, compare);
  __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, compare);
  __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, compare);
  __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_4, compare);
}
```

#### 输入捕获原理（TIM2）

```
FG信号 = 风扇转速反馈信号，每转产生2个脉冲

频率计算：
- FG频率 = 脉冲数 / 时间
- 风扇转速（RPM）= FG频率 * 60 / 2

例如：
- FG频率 = 50Hz
- 风扇转速 = 50 * 60 / 2 = 1500 RPM
```

---

### 4. I2C模块（TLA2528 ADC）

#### 文件位置
- 头文件：`Inc/i2c.h`
- 源文件：`Src/i2c.c`

#### 功能描述
I2C模块用于与TLA2528 ADC芯片通信，读取NTC温度传感器数据。

#### I2C工作原理

```
I2C总线特点：
- 两线制：SCL（时钟线）、SDA（数据线）
- 主从结构：STM32为主设备，TLA2528为从设备
- 7位地址：TLA2528地址为 0x84（7位）或 0x10（8位）

数据传输格式：
START + 从设备地址(W) + 寄存器地址 + 数据 + STOP
```

#### I2C初始化

```c
// i2c.c - MX_I2C_Init() 函数

void MX_I2C_Init(void)
{
  hi2c.Instance = I2C2;  // 选择I2C2外设
  hi2c.Init.ClockSpeed = 50000;  // 时钟频率：50kHz（降低以提高稳定性）
  hi2c.Init.DutyCycle = I2C_DUTYCYCLE_2;  // 占空比：2
  hi2c.Init.OwnAddress1 = 0;  // 自身地址（主模式无需设置）
  hi2c.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;  // 7位地址模式
  hi2c.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;  // 双地址模式：禁用
  hi2c.Init.OwnAddress2 = 0;
  hi2c.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;  // 通用呼叫：禁用
  hi2c.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;  // 时钟拉伸：启用
  
  if (HAL_I2C_Init(&hi2c) != HAL_OK) {
    Error_Handler();
  }
}
```

#### TLA2528寄存器操作

```c
// i2c.c - TLA2528_WriteReg() 函数：写寄存器

HAL_StatusTypeDef TLA2528_WriteReg(uint8_t reg_addr, uint8_t data)
{
  uint8_t tx_data[3];
  tx_data[0] = (reg_addr << 1) | 0x00;  // 寄存器地址 + 写标志
  tx_data[1] = (data << 1) | 0x00;      // 数据 + 写标志
  tx_data[2] = 0x00;                    // 补齐字节
  
  return HAL_I2C_Master_Transmit(&hi2c, TLA2528_I2C_WRITE_ADDR, 
                                  tx_data, 3, 100);
}

// i2c.c - TLA2528_ReadReg() 函数：读寄存器

HAL_StatusTypeDef TLA2528_ReadReg(uint8_t reg_addr, uint8_t *data)
{
  uint8_t tx_data[2];
  tx_data[0] = (reg_addr << 1) | 0x00;  // 寄存器地址 + 写标志
  tx_data[1] = 0x00;                    // 补齐字节
  
  // 1. 发送寄存器地址
  HAL_I2C_Master_Transmit(&hi2c, TLA2528_I2C_WRITE_ADDR, tx_data, 2, 100);
  
  // 2. 读取数据
  return HAL_I2C_Master_Receive(&hi2c, TLA2528_I2C_READ_ADDR, data, 1, 100);
}
```

#### NTC温度计算

```c
// main.c - NTC温度读取

// 1. 读取ADC值
uint16_t adc_value = 0;
TLA2528_ReadChannel(NTC_CHANNEL, &adc_value, NULL);

// 2. 计算电压
float v_ntc = (float)adc_value / NTC_ADC_MAX * NTC_VCC;  // NTC_VCC = 3.3V

// 3. 计算电阻
float r_ntc = 0.0f;
if (NTC_VCC - v_ntc > 0.001f) {
    r_ntc = (NTC_RREF * v_ntc) / (NTC_VCC - v_ntc);  // NTC_RREF = 10000Ω
}

// 4. 通过电阻查表或公式计算温度
// （此处省略温度计算公式）
```

---

## USART2通信运行顺序详解

### 1. 初始化流程（从main函数开始）

#### 步骤1：调用MX_USART2_UART_Init()

**位置**：`main.c` 第715行
```c
MX_USART2_UART_Init();  // 初始化USART2
```

**调用链**：
```
main() → MX_USART2_UART_Init() → HAL_UART_Init() → HAL_UART_MspInit()
```

#### 步骤2：MX_USART2_UART_Init()函数执行

**位置**：`usart.c` 第63-88行

```c
void MX_USART2_UART_Init(void)
{
  huart2.Instance = USART2;  // 选择USART2外设
  huart2.Init.BaudRate = 9600;  // 波特率：9600
  huart2.Init.WordLength = UART_WORDLENGTH_8B;  // 数据位：8位
  huart2.Init.StopBits = UART_STOPBITS_1;  // 停止位：1位
  huart2.Init.Parity = UART_PARITY_NONE;  // 校验位：无
  huart2.Init.Mode = UART_MODE_TX_RX;  // 模式：收发模式
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;  // 硬件流控：禁用
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;  // 过采样：16倍
  
  if (HAL_UART_Init(&huart2) != HAL_OK) {
    Error_Handler();  // 初始化失败处理
  }
}
```

#### 步骤3：HAL_UART_MspInit()初始化硬件资源

**位置**：`usart.c` 第91-165行

```c
void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  
  if(uartHandle->Instance==USART2)  // 判断是否为USART2
  {
    // 1. 使能USART2时钟
    __HAL_RCC_USART2_CLK_ENABLE();
    
    // 2. 使能GPIOA时钟
    __HAL_RCC_GPIOA_CLK_ENABLE();
    
    // 3. 配置PA2为USART2_TX（复用推挽输出）
    GPIO_InitStruct.Pin = GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;  // 复用推挽输出
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    // 4. 配置PA3为USART2_RX（输入）
    GPIO_InitStruct.Pin = GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;  // 输入模式
    GPIO_InitStruct.Pull = GPIO_NOPULL;  // 无上下拉
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    // 5. 配置USART2中断优先级
    HAL_NVIC_SetPriority(USART2_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USART2_IRQn);  // 使能USART2中断
  }
}
```

**引脚配置说明**：
- **PA2**：USART2_TX（发送引脚），配置为复用推挽输出
- **PA3**：USART2_RX（接收引脚），配置为输入（无上下拉）

#### 步骤4：启动中断接收

**位置**：`main.c` 第834-836行

```c
/* 启动USART2中断接收 */
extern uint8_t rx_buffer_usart2[];
extern volatile uint8_t rx_index_usart2;
rx_index_usart2 = 0;  // 重置接收索引
HAL_UART_Receive_IT(&huart2, &rx_buffer_usart2[rx_index_usart2], 1);
```

**关键参数说明**：
- `&huart2`：USART2句柄
- `&rx_buffer_usart2[rx_index_usart2]`：接收缓冲区地址
- `1`：每次接收1个字节（字节流模式）

---

### 2. 接收状态机详解

#### 状态机变量定义

**位置**：`usart.c` 第180-185行

```c
// USART2接收缓冲区定义
#define RX_BUFFER_SIZE_USART2 100
uint8_t rx_buffer_usart2[RX_BUFFER_SIZE_USART2];  // 原始接收缓冲区
volatile uint8_t rx_index_usart2 = 0;  // 当前接收索引（接收过程中使用）
static uint8_t cmd_buffer_usart2[RX_BUFFER_SIZE_USART2];  // 完整命令缓冲区
volatile uint8_t usart2_cmd_ready = 0;  // 命令就绪标志
```

**状态机状态**：

| 状态 | 变量值 | 说明 |
|------|--------|------|
| 初始状态 | `rx_index_usart2 = 0` | 等待第一个字节 |
| 接收中 | `rx_index_usart2 > 0` | 正在接收数据 |
| 命令就绪 | `usart2_cmd_ready = 1` | 完整命令已接收 |
| 命令处理中 | `usart2_cmd_ready = 0` | 命令正在被处理 |

#### 接收状态机流程图

```
┌─────────────────────────────────────────────────────────────────┐
│                         USART2接收状态机                         │
└─────────────────────────────────────────────────────────────────┘

1. 初始化
   ↓
   HAL_UART_Receive_IT(&huart2, &rx_buffer_usart2[0], 1)
   ↓
2. 等待中断（空闲状态）
   ↓
3. 收到第1个字节 → USART2_IRQHandler() → HAL_UART_IRQHandler()
   ↓
   HAL_UART_RxCpltCallback() 被调用
   ↓
4. 判断是否为结束符（\r 或 \n）
   ├─ 是 → 复制到cmd_buffer_usart2，设置usart2_cmd_ready=1，重置rx_index_usart2=0
   │       ↓
   │       启动下一次接收：HAL_UART_Receive_IT(&huart2, &rx_buffer_usart2[0], 1)
   │
   └─ 否 → rx_index_usart2++，继续接收下一个字节
           ↓
           HAL_UART_Receive_IT(&huart2, &rx_buffer_usart2[rx_index_usart2], 1)
           ↓
           返回步骤3
```

#### HAL_UART_RxCpltCallback()回调函数详解

**位置**：`usart.c` 第218-283行

```c
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == USART2)  // 判断是否为USART2中断
  {
    // 检查是否收到结束符（\r 或 \n）
    if (rx_buffer_usart2[rx_index_usart2] == '\r' || 
        rx_buffer_usart2[rx_index_usart2] == '\n')
    {
      // 确保至少有一个有效字符
      if (rx_index_usart2 > 0)
      {
        // 复制完整命令到独立缓冲区
        if (!usart2_cmd_ready)  // 防止命令被覆盖
        {
          uint8_t copy_len = rx_index_usart2;  // 获取实际长度
          if (copy_len >= RX_BUFFER_SIZE_USART2) 
            copy_len = RX_BUFFER_SIZE_USART2 - 1;
          
          // 复制数据
          memcpy(cmd_buffer_usart2, rx_buffer_usart2, copy_len);
          cmd_buffer_usart2[copy_len] = '\0';  // 添加字符串结束符
          
          usart2_cmd_ready = 1;  // 设置命令就绪标志
        }
      }
      
      // 重置索引，准备下一次接收
      rx_index_usart2 = 0;
      
      // 启动下一次接收
      HAL_UART_Receive_IT(&huart2, &rx_buffer_usart2[rx_index_usart2], 1);
    }
    else if (rx_index_usart2 >= RX_BUFFER_SIZE_USART2 - 1)
    {
      // 缓冲区已满，强制结束接收（防止溢出）
      rx_buffer_usart2[rx_index_usart2] = '\0';
      
      if (!usart2_cmd_ready)
      {
        memcpy(cmd_buffer_usart2, rx_buffer_usart2, RX_BUFFER_SIZE_USART2 - 1);
        cmd_buffer_usart2[RX_BUFFER_SIZE_USART2 - 1] = '\0';
        usart2_cmd_ready = 1;
      }
      
      rx_index_usart2 = 0;
      HAL_UART_Receive_IT(&huart2, &rx_buffer_usart2[rx_index_usart2], 1);
    }
    else
    {
      // 继续接收下一个字节
      rx_index_usart2++;
      HAL_UART_Receive_IT(&huart2, &rx_buffer_usart2[rx_index_usart2], 1);
    }
  }
}
```

**回调函数执行时机**：
- 每次成功接收1个字节后，HAL库自动调用此回调函数
- 用户在回调函数中处理接收到的数据

---

### 3. 中断服务程序流程

#### USART2_IRQHandler()中断服务程序

**位置**：`stm32f1xx_it.c` 第225-234行

```c
void USART2_IRQHandler(void)
{
  HAL_UART_IRQHandler(&huart2);  // 调用HAL库中断处理函数
}
```

**HAL_UART_IRQHandler()内部流程**：

```
HAL_UART_IRQHandler()
  ↓
1. 检查RXNE标志（接收非空）
   ↓
2. 读取DR寄存器（数据寄存器）
   ↓
3. 调用 HAL_UART_RxCpltCallback() 回调函数
   ↓
4. 清除RXNE标志
   ↓
5. 返回主程序
```

**中断触发条件**：
- 当USART2接收FIFO中有数据时，RXNE标志置位
- 触发USART2_IRQn中断
- CPU跳转到USART2_IRQHandler执行

---

### 4. 主程序接收处理

#### USART2_GetReceivedData()函数

**位置**：`usart.c` 第321-352行

```c
uint8_t USART2_GetReceivedData(uint8_t *buffer, uint8_t size)
{
  if (!usart2_cmd_ready) return 0;  // 命令未就绪，返回0
  
  uint8_t len = 0;
  // 计算实际接收到的字节数（不包括结束符）
  while (len < RX_BUFFER_SIZE_USART2 && cmd_buffer_usart2[len] != '\0') {
    len++;
  }
  
  // 确保至少有一个有效字符
  if (len > 0)
  {
    if (len > size - 1) len = size - 1;  // 防止缓冲区溢出
    
    // 复制数据到输出缓冲区
    memcpy(buffer, cmd_buffer_usart2, len);
    buffer[len] = '\0';  // 添加结束符
    
    // 重置接收状态
    usart2_cmd_ready = 0;  // 清除就绪标志，允许接收下一条命令
    
    return len;  // 返回接收到的字节数
  }
  
  // 如果没有有效字符，重置接收状态
  usart2_cmd_ready = 0;
  return 0;
}
```

**函数调用流程**：

```
main.c主循环
  ↓
while(1) {
  ↓
  len = USART2_GetReceivedData(rx_data, sizeof(rx_data));
  ↓
  if (len > 0) {
    ↓
    // 命令已就绪，处理命令
    // usart2_cmd_ready 被清零
  }
  ↓
  // 继续下一次循环
}
```

**主循环处理TEC响应**：

**位置**：`main.c` 第991-1010行

```c
/* Check USART2 RX - Process TEC Controller response */
uint8_t rx_data_usart2[32];
uint8_t len_usart2 = USART2_GetReceivedData(rx_data_usart2, sizeof(rx_data_usart2));
if (len_usart2 > 0)
{
  // 打印调试信息
  UART_Printf("[TEC RX] Len: %d, Data: %s\r\n", len_usart2, rx_data_usart2);
  
  // 打印十六进制数据
  UART_SendString("[TEC RX] Hex: ");
  for (uint8_t i = 0; i < len_usart2; i++) {
    UART_Printf("%02X ", rx_data_usart2[i]);
  }
  UART_SendString("\r\n");
  
  // 转发响应到USART1（PC串口助手）
  UART_SendString("[TEC] ");
  UART_SendString((const char *)rx_data_usart2);
  UART_SendString("\r\n");
  
  // 清除等待标志
  tec_wait_response = 0;
}
```

---

### 5. 完整通信流程示例

#### 场景：PC串口助手发送指令" S1 25.5\r\n"

**步骤1：PC发送数据**

```
PC串口助手 → USART1 → MCU → USART2 → TEC控制器
发送：'S' '1' ' ' '2' '5' '.' '5' '\r' '\n'
```

**步骤2：TEC控制器响应**

```
TEC控制器 → USART2 → MCU → USART1 → PC串口助手
响应："OK\r\n"
```

**详细流程图**：

```
┌─────────────────────────────────────────────────────────────────┐
│                    完整通信流程（TEC控制器）                     │
└─────────────────────────────────────────────────────────────────┘

1. PC串口助手发送指令
   ↓
   'S' '1' ' ' '2' '5' '.' '5' '\r' '\n'
   ↓
2. USART1接收（115200bps）
   ↓
   HAL_UART_RxCpltCallback() → rx_buffer[] → cmd_buffer → cmd_ready=1
   ↓
3. main.c主循环处理USART1命令
   ↓
   len = USART1_GetReceivedData(rx_data, sizeof(rx_data));
   if (len > 0) {
     trim_cmd((char *)rx_data);  // 去除空白
     ↓
     // 转发至USART2
     actual_len = strlen((const char *)rx_data);
     UART_Printf("[TEC TX] Cmd: %s, Len: %d\r\n", rx_data, actual_len);
     ↓
     HAL_UART_Transmit(&huart2, rx_data, actual_len, 100);  // 发送指令
     ↓
     crlf[] = "\r\n";
     HAL_UART_Transmit(&huart2, crlf, 2, 100);  // 发送结束符
     ↓
     tec_wait_response = 1;  // 设置等待标志
     tec_cmd_send_time = HAL_GetTick();  // 记录发送时间
   }
   ↓
4. USART2发送完成，自动启动接收
   ↓
   HAL_UART_Receive_IT(&huart2, &rx_buffer_usart2[0], 1)
   ↓
5. TEC控制器响应"OK\r\n"
   ↓
   'O' 'K' '\r' '\n'
   ↓
6. USART2接收中断
   ↓
   USART2_IRQHandler() → HAL_UART_IRQHandler() → HAL_UART_RxCpltCallback()
   ↓
   rx_buffer_usart2[] = {'O', 'K', '\r'}
   rx_index_usart2 = 3
   usart2_cmd_ready = 1
   ↓
7. main.c主循环处理USART2响应
   ↓
   len_usart2 = USART2_GetReceivedData(rx_data_usart2, sizeof(rx_data_usart2));
   if (len_usart2 > 0) {
     UART_Printf("[TEC RX] Len: %d, Data: %s\r\n", len_usart2, rx_data_usart2);
     ↓
     // 转发到USART1
     UART_SendString("[TEC] ");
     UART_SendString((const char *)rx_data_usart2);
     UART_SendString("\r\n");
     ↓
     tec_wait_response = 0;  // 清除等待标志
   }
   ↓
8. PC串口助手收到响应
   ↓
   "[TEC] OK"
```

---

### 6. 超时检测机制

**位置**：`main.c` 第998-1003行

```c
/* Check TEC response timeout (500ms) */
if (tec_wait_response && (HAL_GetTick() - tec_cmd_send_time > 500))
{
  UART_SendString("[TEC RX] Timeout: No response from TEC\r\n");
  tec_wait_response = 0;
}
```

**超时检测逻辑**：
- 发送指令后设置`tec_wait_response = 1`
- 记录发送时间`tec_cmd_send_time = HAL_GetTick()`
- 每次循环检查：当前时间 - 发送时间 > 500ms？
- 如果超时，打印超时信息并清除等待标志

---

### 7. 关键数据结构总结

#### USART2接收相关变量

| 变量名 | 类型 | 作用 | 说明 |
|--------|------|------|------|
| `rx_buffer_usart2[]` | `uint8_t[100]` | 原始接收缓冲区 | 接收过程中使用，易被覆盖 |
| `rx_index_usart2` | `volatile uint8_t` | 接收索引 | 接收过程中递增，中断中使用 |
| `cmd_buffer_usart2[]` | `uint8_t[100]` | 完整命令缓冲区 | 保存完整命令，供主程序读取 |
| `usart2_cmd_ready` | `volatile uint8_t` | 命令就绪标志 | 1=命令就绪，0=命令未就绪 |

#### 通信状态标志

| 标志名 | 位置 | 说明 |
|--------|------|------|
| `usart2_cmd_ready` | `usart.c` | USART2命令就绪标志 |
| `tec_wait_response` | `main.c` | 等待TEC响应标志 |
| `tec_cmd_send_time` | `main.c` | TEC指令发送时间戳 |

---

### 8. 调试技巧

#### 打印调试信息

```c
// 打印接收缓冲区内容
UART_Printf("[USART2 RX] Index=%d, Data=%s\r\n", rx_index_usart2, rx_buffer_usart2);

// 打印命令就绪状态
UART_Printf("[USART2 RX] cmd_ready=%d\r\n", usart2_cmd_ready);

// 打印接收到的十六进制数据
for (uint8_t i = 0; i < len_usart2; i++) {
  UART_Printf("%02X ", rx_data_usart2[i]);
}
UART_SendString("\r\n");
```

#### 常见问题排查

| 问题现象 | 可能原因 | 检查方法 |
|----------|----------|----------|
| 收不到TEC响应 | USART2未启动接收 | 检查`HAL_UART_Receive_IT()`是否调用 |
| 响应数据不完整 | 缓冲区溢出 | 检查`rx_index_usart2`是否达到100 |
| 重复接收相同数据 | `usart2_cmd_ready`未清零 | 检查`USART2_GetReceivedData()`是否清除标志 |
| 超时无响应 | TEC未正确连接 | 检查硬件连接，使用示波器测量TX/RX |

---

## STM32中断系统原理详解

### 1. 中断系统概述

STM32的中断系统是**硬件+软件**协同工作的结果：

```
硬件自动完成的部分：
├─ RXNE标志设置（USART接收数据时自动置位）
├─ 中断请求产生（NVIC挂起中断）
└─ 向量表查找和跳转（CPU自动完成）

软件配置的部分：
├─ 中断使能配置（HAL_NVIC_EnableIRQ）
├─ 中断服务程序实现（USART2_IRQHandler）
├─ HAL库中断处理（HAL_UART_IRQHandler）
└─ 用户回调函数（HAL_UART_RxCpltCallback）
```

---

### 2. 中断向量表（Vector Table）

#### 2.1 向量表地址

**定义位置**：`startup_stm32f103xe.s`

```assembly
; Vector Table Mapped to Address 0 at Reset
                AREA    RESET, DATA, READONLY
                EXPORT  __Vectors          ; ← 向量表起始地址 = 0x0000_0000

__Vectors       DCD     __initial_sp       ; 地址 0x0000_0000 (栈顶)
                DCD     Reset_Handler      ; 地址 0x0000_0004 (复位处理)
                DCD     NMI_Handler        ; 地址 0x0000_0008 (NMI)
                ; ...
```

**关键点**：
- `__Vectors` = 0x0000_0000（Flash起始地址）
- 向量表位于地址0，上电后CPU自动从这里开始执行

#### 2.2 向量表条目结构

```
每个条目 = 4字节（32位地址）
DCD = Define Constant Doubleword（定义32位常量）

条目0: 0x0000_0000, __initial_sp      (栈顶地址)
条目1: 0x0000_0004, Reset_Handler      (复位处理)
条目2: 0x0000_0008, NMI_Handler        (NMI处理)
...
条目37: 0x0000_0094, USART1_IRQHandler  (USART1中断)
条目38: 0x0000_0098, USART2_IRQHandler  (USART2中断) ← 您问的
条目39: 0x0000_009C, USART3_IRQHandler  (USART3中断)
```

#### 2.3 地址计算公式

```
地址 = 0x0000_0000 + (_IRQn * 4)

USART2_IRQn = 38
地址 = 0x0000_0000 + (38 * 4) = 0x0000_0098
```

**为什么乘以4？**
- ARM Cortex-M3是32位处理器
- 每个地址存放1个字节（8位）
- 一个32位地址需要4个字节
- 所以每个向量占用4个字节

---

### 3. 中断号定义

**文件位置**：`Drivers/CMSIS/Device/ST/STM32F1xx/Include/stm32f103xe.h`

```c
typedef enum
{
  // Cortex-M3内部中断（负数）
  NonMaskableInt_IRQn         = -14,    // 0-1: NMI
  HardFault_IRQn              = -13,    // 2-3: Hard Fault
  // ...
  
  // STM32外设中断（正数）
  WWDG_IRQn                   = 0,      // 16-17: Window Watchdog
  PVD_IRQn                    = 1,      // 17-18: PVD
  // ...
  USART1_IRQn                 = 37,     // 53-54: USART1
  USART2_IRQn                 = 38,     // 54-55: USART2 ← 这里定义
  USART3_IRQn                 = 39,     // 55-56: USART3
  // ...
} IRQn_Type;
```

**关键点**：
- `USART2_IRQn = 38` 是一个常量定义
- 用于计算向量表地址
- 在整个系统中唯一标识USART2中断

---

### 4. 中断触发完整流程

#### 4.1 硬件自动完成的步骤

```
步骤1：外部设备发送数据
       ↓
       TEC控制器 → PA3 (USART2_RX)

步骤2：USART2接收数据
       ↓
       数据移入DR寄存器

步骤3：硬件自动设置RXNE标志
       ↓
       USART2->SR |= USART_SR_RXNE (0x20)
       ↓
       RXNE = 1 (硬件自动设置，不需要软件代码)

步骤4：检查中断使能
       ↓
       如果 USART2->CR1 & USART_CR1_RXNEIE != 0
       且 PRIMASK = 0 (全局中断使能)

步骤5：产生中断请求
       ↓
       NVIC挂起USART2_IRQn = 38

步骤6：CPU检测到中断
       ↓
       PC = 0x0000_0000 + (38 * 4) = 0x0000_0098

步骤7：读取向量表
       ↓
       读取地址0x0000_0098的值
       该值是 USART2_IRQHandler 函数的地址

步骤8：跳转执行
       ↓
       PC = USART2_IRQHandler地址
       开始执行中断服务程序
```

#### 4.2 软件配置的步骤

**步骤1：配置USART2中断使能**

**位置**：`Src/usart.c` 第105-115行

```c
void MX_USART2_UART_Init(void)
{
  // ...
  
  // 使能USART2接收中断
  USART2->CR1 |= USART_CR1_RXNEIE;  // ← 使能RXNE中断
}
```

**步骤2：配置NVIC中断控制器**

**位置**：`Src/usart.c` 第157行

```c
void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{
  if(uartHandle->Instance==USART2)
  {
    // ...
    
    // 配置NVIC
    HAL_NVIC_SetPriority(USART2_IRQn, 0, 0);  // 设置优先级
    HAL_NVIC_EnableIRQ(USART2_IRQn);          // 使能USART2中断
  }
}
```

**HAL_NVIC_EnableIRQ()函数实现**：

```c
static inline void HAL_NVIC_EnableIRQ(IRQn_Type IRQn)
{
  if ((int32_t)IRQn >= 0) {
    // NVIC_ISER[1] 控制中断32-63
    // USART2_IRQn = 38, 38 = 32 + 6
    // 所以设置 NVIC_ISER[1] 的第6位
    NVIC->ISER[((uint32_t)IRQn >> 5UL)] = 
      (uint32_t)(1UL << ((uint32_t)IRQn & 0x1FUL));
  }
}
```

**NVIC_ISER寄存器说明**：

```
NVIC_ISER[0] (0xE000_E100) - 控制中断0-31
NVIC_ISER[1] (0xE000_E104) - 控制中断32-63 ← USART2在这里

USART2_IRQn = 38
38 = 32 + 6
所以设置 NVIC_ISER[1] 的第6位
```

**步骤3：实现中断服务程序**

**位置**：`Src/stm32f1xx_it.c` 第225-234行

```c
void USART2_IRQHandler(void)
{
  HAL_UART_IRQHandler(&huart2);  // 调用HAL库处理
}
```

**位置**：`Src/usart.c` 第218-283行

```c
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == USART2)
  {
    // 处理接收到的数据
  }
}
```

---

### 5. DCD和函数地址的关系

#### 5.1 DCD的含义

```assembly
; 启动文件中的定义
__Vectors       DCD     __initial_sp
                DCD     Reset_Handler
                DCD     NMI_Handler
                ; ...
                DCD     USART2_IRQHandler  ; ← 这里
```

**DCD = Define Constant Doubleword**
- D = Define (定义)
- C = Constant (常量)
- D = Doubleword (双字，32位)

#### 5.2 标签转换为地址

```assembly
; USART2_IRQHandler 是一个标签（label）
; 链接器会将标签转换为实际地址

; 假设 USART2_IRQHandler 函数地址是 0x0800_0123
DCD USART2_IRQHandler

; 链接后实际变成：
DCD 0x08000123

; 在内存中存储为（小端序）：
; 地址 0x0000_0098: 0x23
; 地址 0x0000_0099: 0x01
; 地址 0x0000_009A: 0x00
; 地址 0x0000_009B: 0x08
```

#### 5.3 函数实现

**位置**：`Src/stm32f1xx_it.c`

```c
void USART2_IRQHandler(void)  // ← 函数实现
{
  HAL_UART_IRQHandler(&huart2);
}
```

**位置**：`Src/usart.c`

```c
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == USART2)
  {
    // 处理接收数据
  }
}
```

---

### 6. 完整的地址映射

```
链接后实际内存布局：

Flash地址空间 (0x0800_0000):
0x0800_0000:  程序代码开始
...
0x0800_0123:  USART2_IRQHandler函数开始 ← 函数的实际地址
              B       .  ; 无限循环（占位符）
              ...

向量表地址空间 (0x0000_0000):
0x0000_0000:  0x20 0x04 0x00 0x20  __initial_sp (栈顶地址)
0x0000_0004:  0x01 0x00 0x00 0x08  Reset_Handler地址
...
0x0000_0098:  0x23 0x01 0x00 0x08  USART2_IRQHandler地址 ← 您问的
              (小端序：0x08000123)
0x0000_009C:  0xXX 0xXX 0xXX 0xXX  USART3_IRQHandler地址
```

---

### 7. CPU执行流程详解

#### 7.1 复位流程

```
1. 上电复位
   ↓
   PC = 0x0000_0000 (复位向量)

2. 读取复位向量
   ↓
   地址 0x0000_0000: __initial_sp (栈顶地址)
   地址 0x0000_0004: Reset_Handler (复位处理函数)

3. 跳转到Reset_Handler
   ↓
   Reset_Handler开始执行
   - 初始化RAM
   - 调用SystemInit()
   - 调用main()

4. main()执行
   ↓
   - HAL_Init()
   - MX_GPIO_Init()
   - MX_USART2_UART_Init()
   - HAL_NVIC_EnableIRQ(USART2_IRQn) ← 使能USART2中断
   - while(1) { ... } ← 进入主循环
```

#### 7.2 中断触发流程

```
1. 外部设备发送数据
   ↓
   TEC控制器 → PA3 (USART2_RX)

2. USART2接收数据
   ↓
   数据移入DR寄存器

3. 硬件自动设置RXNE标志
   ↓
   USART2->SR |= USART_SR_RXNE (0x20)
   RXNE = 1

4. 检查中断使能
   ↓
   USART2->CR1 & USART_CR1_RXNEIE != 0  ← 已使能
   PRIMASK = 0  ← 全局中断未屏蔽

5. 产生中断请求
   ↓
   NVIC挂起USART2_IRQn = 38

6. CPU检测到中断
   ↓
   PC = 0x0000_0000 + (38 * 4) = 0x0000_0098

7. 读取向量表
   ↓
   读取地址0x0000_0098的值
   读取到 0x0800_0123 (USART2_IRQHandler地址)

8. 跳转执行
   ↓
   PC = 0x0800_0123
   开始执行 USART2_IRQHandler 函数

9. 执行中断服务程序
   ↓
   void USART2_IRQHandler(void)
   {
     HAL_UART_IRQHandler(&huart2);
   }

10. HAL库处理
    ↓
    HAL_UART_IRQHandler() 内部执行：
    - 检查RXNE标志
    - 读取DR寄存器
    - 调用回调函数

11. 执行回调函数
    ↓
    void HAL_UART_RxCpltCallback()
    {
      if (huart->Instance == USART2) {
        // 处理接收数据
      }
    }

12. 返回主程序
    ↓
    恢复上下文（POP）
    返回到被中断的主程序
```

---

### 8. 关键代码文件汇总

| 文件 | 作用 | 关键内容 |
|------|------|----------|
| `startup_stm32f103xe.s` | 中断向量表 | 定义USART2_IRQHandler地址 |
| `stm32f103xe.h` | 中断号定义 | `USART2_IRQn = 38` |
| `stm32f1xx_it.c` | 中断服务程序 | 实现`USART2_IRQHandler()` |
| `usart.c` | 用户回调函数 | 实现`HAL_UART_RxCpltCallback()` |
| `stm32f1xx_hal_uart.c` | HAL库内部 | `HAL_UART_IRQHandler()`实现 |
| `core_cm3.h` | Cortex-M3内核 | `__get_PRIMASK()`等 |

---

### 9. 关键概念总结

#### 9.1 硬件自动完成的部分（不需要代码）

| 功能 | 说明 |
|------|------|
| RXNE标志设置 | 硬件自动设置，数据移入DR后自动置1 |
| 中断请求产生 | NVIC自动挂起中断 |
| 向量表查找 | CPU自动计算地址并读取 |
| 上下文保存 | CPU自动压栈保存寄存器 |
| 中断跳转 | CPU自动设置PC寄存器 |

#### 9.2 软件配置的部分（需要代码）

| 功能 | 说明 |
|------|------|
| 中断使能配置 | `HAL_NVIC_EnableIRQ()` |
| 中断服务程序 | `USART2_IRQHandler()` |
| HAL库处理 | `HAL_UART_IRQHandler()` |
| 用户回调函数 | `HAL_UART_RxCpltCallback()` |

#### 9.3 关键地址计算

```
USART2_IRQn = 38

向量表地址 = 0x0000_0000 + (38 * 4) = 0x0000_0098

为什么乘以4？
- ARM Cortex-M3是32位处理器
- 每个地址存放1个字节（8位）
- 一个32位地址需要4个字节
- 所以每个向量占用4个字节
```

---

### 10. 常见问题

#### Q1: RXNE标志是谁设置的？

**A**：硬件自动设置
- 当USART2接收数据寄存器DR中有新数据时
- 硬件自动将RXNE位置1
- 不需要任何软件代码

#### Q2: 中断向量表在哪里？

**A**：在启动文件中
- **文件**：`startup_stm32f103xe.s`
- **位置**：`__Vectors` 标签开始
- **地址**：0x0000_0000（Flash起始地址）

#### Q3: 为什么读取0x0000_0098就能启动中断函数？

**A**：因为0x0000_0098处存储的是函数地址
- DCD USART2_IRQHandler 定义的是函数的入口地址
- 链接器将标签转换为实际地址
- CPU读取后设置PC寄存器，开始执行函数

#### Q4: 是CPU自己查找向量表还是软件查找？

**A**：CPU硬件自动查找
- 不需要软件代码
- CPU内部的中断控制器自动处理
- 根据中断号计算地址：`地址 = 0x0000_0000 + (_IRQn * 4)`

#### Q5: USART2_IRQHandler在哪个文件？

**A**：多个文件共同完成
- **定义**：`startup_stm32f103xe.s` (向量表)
- **实现**：`stm32f1xx_it.c` (中断服务程序)
- **处理**：`usart.c` (回调函数)

---

## 代码阅读指南

### 阅读步骤（从上到下，从简到繁）

#### 第一步：理解项目整体架构

1. **阅读 main.h**
   - 了解所有引脚定义（如 `LED2_Pin`、`UART1_TX_Pin` 等）
   - 了解所有外设配置参数（如 `NTC_CHANNEL`、`TLA2528_I2C_ADDR` 等）

2. **阅读 main.c 的 main() 函数**
   - 理解初始化流程（HAL_Init → SystemClock_Config → MX_GPIO_Init → ...）
   - 理解主循环逻辑（串口接收 → 指令解析 → 数据转发）

#### 第二步：逐个模块深入

**推荐阅读顺序**：

```
1. gpio.c / gpio.h      → 理解引脚配置
2. usart.c / usart.h    → 理解串口通信
3. tim.c / tim.h        → 理解定时器/PWM
4. i2c.c / i2c.h        → 理解I2C通信
5. main.c (完整)        → 理解整体逻辑
```

#### 第三步：理解函数调用链

**示例：PWM占空比修改流程**

```
main.c: FANPWM_SetDutyPercent(duty)
    ↓
main.c: __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, compare)
    ↓
tim.h: __HAL_TIM_SET_COMPARE 宏定义
    ↓
stm32f1xx_hal_tim.h: 直接写TIM3->CCR1寄存器
```

**示例：串口接收流程**

```
PC串口助手发送数据
    ↓
USART1_IRQHandler() 中断服务程序
    ↓
HAL_UART_IRQHandler() HAL库中断处理
    ↓
HAL_UART_RxCpltCallback() 用户回调函数
    ↓
main.c: USART1_GetReceivedData() 获取完整命令
    ↓
main.c: trim_cmd() 去除空白
    ↓
main.c: equals_ignore_case() 指令匹配
    ↓
main.c: 执行对应功能
```

#### 第四步：理解数据结构

**GPIO配置结构体**

```c
typedef struct {
  uint32_t Pin;       // 引脚号（如 GPIO_PIN_5）
  uint32_t Mode;      // 模式（如 GPIO_MODE_OUTPUT_PP）
  uint32_t Pull;      // 上下拉（如 GPIO_NOPULL）
  uint32_t Speed;     // 速度（如 GPIO_SPEED_FREQ_LOW）
} GPIO_InitTypeDef;
```

**UART配置结构体**

```c
typedef struct {
  uint32_t BaudRate;         // 波特率
  uint32_t WordLength;       // 数据位
  uint32_t StopBits;         // 停止位
  uint32_t Parity;           // 校验位
  uint32_t Mode;             // 模式（TX/RX）
  uint32_t HwFlowCtl;        // 硬件流控
  uint32_t OverSampling;     // 过采样
} UART_InitTypeDef;
```

---

## TIM输入捕获原理详解

### 1. 输入捕获概述

**功能**：测量外部信号的频率、占空比、脉宽等参数

**本项目应用**：使用TIM2_CH2（PA1）捕获风扇FG（转速反馈）信号

**工作原理**：
- 当PA1引脚检测到上升沿（或下降沿）时
- 自动将当前定时器计数值（CNT）存入捕获寄存器（CCR）
- 触发中断，通知CPU处理
- 通过计算两次捕获值的差值，得到信号周期

---

### 2. 完整调用流程

#### 2.1 初始化流程

```
main() 函数
    ↓
MX_TIM2_Init()  // tim.c 第61行
    ↓
HAL_TIM_IC_Init(&htim2)  // HAL库函数
    ↓
HAL_TIM_IC_MspInit(&htim2)  // MSP初始化
    ↓
MX_TIM2_MspInit()  // GPIO和NVIC配置
```

#### 2.2 启动流程

```
main() 函数第749行
    ↓
HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_2)  // 启动输入捕获中断
    ↓
HAL库内部配置
    ↓
TIM2_IRQHandler() 中断服务程序
    ↓
HAL_TIM_IRQHandler(&htim2)  // HAL库中断处理
    ↓
HAL_TIM_IC_CaptureCallback(&htim2)  // 用户回调函数
```

---

### 3. 逐行代码详解

#### 3.1 初始化函数 - MX_TIM2_Init()

**位置**：`tim.c` 第61-98行

```c
void MX_TIM2_Init(void)
{
  /* USER CODE BEGIN TIM2_Init 0 */
  /* USER CODE END TIM2_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_IC_InitTypeDef sConfigIC = {0};  // ← 输入捕获配置结构体

  /* USER CODE BEGIN TIM2_Init 1 */
  /* USER CODE END TIM2_Init 1 */
  
  htim2.Instance = TIM2;  // ← 选择TIM2外设
  
  // 系统时钟是64MHz，APB1分频2，所以TIM2时钟是64MHz
  // 64MHz / 6400 = 10kHz 计数频率
  htim2.Init.Prescaler = 6399;  // ← 预分频器：64MHz / 6400 = 10kHz
  // 预分频器计算：
  // - 系统时钟 = 64MHz
  // - PSC = 6399 (实际分频 = PSC + 1 = 6400)
  // - 计数频率 = 64MHz / 6400 = 10kHz
  // - 计数周期 = 1 / 10kHz = 0.1ms = 100μs
  
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;  // ← 计数模式：向上计数
  // 向上计数：CNT从0递增到ARR，然后重载
  // 计数范围：0 ~ 65535 (16位定时器)
  
  htim2.Init.Period = 65535;  // ← 自动重载值：ARR = 65535
  // 当CNT达到ARR时，产生更新事件，CNT重载为0
  // 最大计数值：65536 (0 ~ 65535)
  
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;  // ← 时钟分频：不分频
  // 可选值：TIM_CLOCKDIVISION_DIV1, DIV2, DIV4
  // 分频后作为死区时间和数字滤波器的时钟
  
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;  // ← 自动重载预加载：禁用
  // 禁用：ARR值立即生效
  // 启用：ARR值在更新事件时才生效
  
  if (HAL_TIM_IC_Init(&htim2) != HAL_OK)  // ← 初始化输入捕获模式
  {
    Error_Handler();  // 初始化失败，进入错误处理
  }
  
  // ... (配置主从模式，本项目未使用)
  
  // 配置输入捕获通道2
  sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;  // ← 捕获边沿：上升沿
  // 可选值：
  // - TIM_INPUTCHANNELPOLARITY_RISING: 上升沿捕获
  // - TIM_INPUTCHANNELPOLARITY_FALLING: 下降沿捕获
  // - TIM_INPUTCHANNELPOLARITY_BOTHEDGE: 双边沿捕获
  
  sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;  // ← 输入选择：直接输入
  // 可选值：
  // - TIM_ICSELECTION_DIRECTTI: 直接输入（ICx映射到TIx）
  // - TIM_ICSELECTION_INDIRECTTI: 间接输入（ICx映射到TIy）
  // - TIM_ICSELECTION_TRC: 触发输入（ICx映射到TRC）
  
  sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;  // ← 输入捕获预分频器：不分频
  // 可选值：TIM_ICPSC_DIV1, DIV2, DIV4, DIV8
  // 每N个捕获事件才更新一次CCRx寄存器
  // DIV1: 每个边沿都捕获
  // DIV2: 每2个边沿捕获一次
  // DIV4: 每4个边沿捕获一次
  // DIV8: 每8个边沿捕获一次
  
  sConfigIC.ICFilter = 15;  // ← 数字滤波器：15个采样点
  // 滤波器长度：0-15
  // 0: 不滤波
  // 15: 15个采样点一致才认为是有效边沿
  // 作用：滤除高频噪声和毛刺
  // 采样时钟：fCK_INT (定时器时钟)
  // 滤波器长度 = N × fCK_INT
  // 例如：N=15, fCK_INT=10kHz → 滤波长度 = 15 × 0.1ms = 1.5ms
  
  if (HAL_TIM_IC_ConfigChannel(&htim2, &sConfigIC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  
  /* USER CODE BEGIN TIM2_Init 2 */
  /* USER CODE END TIM2_Init 2 */
}
```

**代码详解**：

1. **htim2.Instance = TIM2**
   - 选择TIM2外设
   - `Instance`是`TIM_HandleTypeDef`结构体的第一个成员
   - 指向定时器的基地址（TIM2 = 0x4000_0000）

2. **Prescaler = 6399**
   - 分频系数 = PSC + 1 = 6400
   - 计数频率 = 64MHz / 6400 = 10kHz
   - 计数周期 = 100μs
   - 例如：FG信号1kHz（600RPM），每个周期1ms = 10个计数

3. **ICPolarity = RISING**
   - 上升沿触发捕获
   - FG信号是方波，上升沿对应转速信息

4. **ICFilter = 15**
   - 数字滤波器长度15
   - 滤除高频噪声
   - 避免误触发

5. **TIM_CHANNEL_2**
   - 使用通道2（CH2）
   - 对应引脚：PA1

---

#### 3.2 MSP初始化 - HAL_TIM_IC_MspInit()

**位置**：`tim.c` 第240-271行

```c
void HAL_TIM_IC_MspInit(TIM_HandleTypeDef* tim_icHandle)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  
  if(tim_icHandle->Instance==TIM2)  // ← 判断是否是TIM2
  {
    /* USER CODE BEGIN TIM2_MspInit 0 */
    /* USER CODE END TIM2_MspInit 0 */
    
    /* TIM2 clock enable */  // ← 使能TIM2时钟
    __HAL_RCC_TIM2_CLK_ENABLE();
    // 宏定义：#define __HAL_RCC_TIM2_CLK_ENABLE() (RCC->APB1ENR |= RCC_APB1ENR_TIM2EN)
    // 设置APB1ENR寄存器的TIM2EN位，使能TIM2时钟
    
    __HAL_RCC_GPIOA_CLK_ENABLE();  // ← 使能GPIOA时钟
    // 使能GPIOA的时钟，才能配置PA1
    
    /**TIM2 GPIO Configuration
    PA1     ------> TIM2_CH2 (FG信号输入)
    */
    GPIO_InitStruct.Pin = GPIO_PIN_1;  // ← 配置PA1引脚
    // 选择PA1引脚
    
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;  // ← 配置为输入模式
    // GPIO_MODE_INPUT: 普通输入模式
    // 因为是输入捕获，不需要输出功能
    
    GPIO_InitStruct.Pull = GPIO_NOPULL;  // ← 无上下拉电阻
    // GPIO_NOPULL: 不使用上下拉
    // GPIO_PULLUP: 上拉电阻
    // GPIO_PULLDOWN: 下拉电阻
    // FG信号为480mV-3.1V，不需要上下拉
    
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);  // ← 初始化GPIOA

    /* TIM2 interrupt Init */  // ← 配置TIM2中断
    HAL_NVIC_SetPriority(TIM2_IRQn, 2, 0);  // ← 设置中断优先级
    // 参数1：TIM2_IRQn - 中断号
    // 参数2：2 - 抢占优先级
    // 参数3：0 - 响应优先级（子优先级）
    // 优先级分组：默认4位抢占优先级，0位响应优先级
    // 优先级计算：(抢占优先级 << 2) | (响应优先级)
    // TIM2_IRQn = 29 (在stm32f1xx.h中定义)
    
    HAL_NVIC_EnableIRQ(TIM2_IRQn);  // ← 使能TIM2中断
    // 设置NVIC_ISER寄存器，允许TIM2中断
    // 宏定义：#define __NVIC_EnableIRQ(IRQn) NVIC->ISER[((uint32_t)(IRQn) >> 5)] = (uint32_t)(1 << ((uint32_t)(IRQn) & (uint32_t)0x1F))
    
    /* USER CODE BEGIN TIM2_MspInit 1 */
    /* USER CODE END TIM2_MspInit 1 */
  }
}
```

**代码详解**：

1. **__HAL_RCC_TIM2_CLK_ENABLE()**
   - 使能TIM2时钟
   - 必须先使能外设时钟，才能配置外设
   - 写寄存器：`RCC->APB1ENR |= RCC_APB1ENR_TIM2EN`

2. **HAL_GPIO_Init(GPIOA, &GPIO_InitStruct)**
   - 初始化GPIOA
   - 配置PA1为输入模式
   - 配置引脚的电气特性（上下拉、速度等）

3. **HAL_NVIC_SetPriority(TIM2_IRQn, 2, 0)**
   - 设置TIM2中断优先级
   - 抢占优先级 = 2
   - 响应优先级 = 0
   - 写寄存器：`NVIC->IP[29] = (2 << 4) | 0`

4. **HAL_NVIC_EnableIRQ(TIM2_IRQn)**
   - 使能TIM2中断
   - 写寄存器：`NVIC->ISER[0] |= (1 << 29)`

---

#### 3.3 启动输入捕获 - main.c

**位置**：`main.c` 第749-755行

```c
/* 启动TIM2输入捕获中断（PA1 - TIM2_CH2） */
if (HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_2) != HAL_OK)  // ← 启动输入捕获中断
{
  UART_SendString("[ERROR] Failed to start TIM2 IC interrupt\r\n");
}
else
{
  UART_SendString("[INFO] TIM2 CH2 (PA1) input capture started\r\n");
}
```

**HAL_TIM_IC_Start_IT()函数详解**：

**位置**：`Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_tim.c` 第2200行

```c
HAL_StatusTypeDef HAL_TIM_IC_Start_IT(TIM_HandleTypeDef *htim, uint32_t Channel)
{
  HAL_StatusTypeDef status = HAL_OK;
  uint32_t tmpsmcr;

  // 1. 检查参数有效性
  HAL_TIM_ChannelStateTypeDef channel_state = TIM_CHANNEL_STATE_GET(htim, Channel);
  HAL_TIM_ChannelStateTypeDef complementary_channel_state = TIM_CHANNEL_N_STATE_GET(htim, Channel);

  if ((channel_state != HAL_TIM_CHANNEL_STATE_READY)
      || (complementary_channel_state != HAL_TIM_CHANNEL_STATE_READY))
  {
    return HAL_ERROR;  // 通道未准备好，返回错误
  }

  // 2. 设置通道状态为忙
  TIM_CHANNEL_STATE_SET(htim, Channel, HAL_TIM_CHANNEL_STATE_BUSY);
  TIM_CHANNEL_N_STATE_SET(htim, Channel, HAL_TIM_CHANNEL_STATE_BUSY);

  // 3. 根据通道使能相应的中断
  switch (Channel)
  {
    case TIM_CHANNEL_1:
    {
      // 使能捕获/比较1中断
      __HAL_TIM_ENABLE_IT(htim, TIM_IT_CC1);
      break;
    }

    case TIM_CHANNEL_2:  // ← 本项目使用通道2
    {
      // 使能捕获/比较2中断
      __HAL_TIM_ENABLE_IT(htim, TIM_IT_CC2);
      break;
    }

    case TIM_CHANNEL_3:
    {
      __HAL_TIM_ENABLE_IT(htim, TIM_IT_CC3);
      break;
    }

    case TIM_CHANNEL_4:
    {
      __HAL_TIM_ENABLE_IT(htim, TIM_IT_CC4);
      break;
    }

    default:
      status = HAL_ERROR;
      break;
  }

  if (status == HAL_OK)
  {
    // 4. 使能输入捕获通道
    TIM_CCxChannelCmd(htim->Instance, Channel, TIM_CCx_ENABLE);
    // 写寄存器：TIM2->CCER |= TIM_CCER_CC2E (使能CC2输入捕获)

    // 5. 使能定时器
    if (IS_TIM_SLAVE_INSTANCE(htim->Instance))
    {
      tmpsmcr = htim->Instance->SMCR & TIM_SMCR_SMS;
      if (!IS_TIM_SLAVEMODE_TRIGGER_ENABLED(tmpsmcr))
      {
        __HAL_TIM_ENABLE(htim);  // 从模式非触发模式，使能定时器
      }
    }
    else
    {
      __HAL_TIM_ENABLE(htim);  // 主模式，使能定时器
    }
    // 写寄存器：TIM2->CR1 |= TIM_CR1_CEN (使能计数器)
  }

  return status;
}
```

**关键操作**：

1. **__HAL_TIM_ENABLE_IT(htim, TIM_IT_CC2)**
   - 使能捕获/比较2中断
   - 写寄存器：`TIM2->DIER |= TIM_DIER_CC2IE`
   - DIER = DMA/中断使能寄存器
   - CC2IE = 捕获/比较2中断使能位

2. **TIM_CCxChannelCmd()**
   - 使能输入捕获通道
   - 写寄存器：`TIM2->CCER |= TIM_CCER_CC2E`
   - CCER = 捕获/比较使能寄存器
   - CC2E = 捕获/比较2输出使能位

3. **__HAL_TIM_ENABLE(htim)**
   - 使能定时器计数器
   - 写寄存器：`TIM2->CR1 |= TIM_CR1_CEN`
   - CR1 = 控制寄存器1
   - CEN = 计数器使能位

---

#### 3.4 中断服务程序 - TIM2_IRQHandler()

**位置**：`stm32f1xx_it.c` 第193-202行

```c
/**
  * @brief This function handles TIM2 global interrupt.
  */
void TIM2_IRQHandler(void)  // ← TIM2全局中断服务程序
{
  /* USER CODE BEGIN TIM2_IRQn 0 */
  /* USER CODE END TIM2_IRQn 0 */
  
  HAL_TIM_IRQHandler(&htim2);  // ← 调用HAL库中断处理函数
  // 参数：&htim2 - TIM2句柄（包含所有配置和状态信息）
  
  /* USER CODE BEGIN TIM2_IRQn 1 */
  /* USER CODE END TIM2_IRQn 1 */
}
```

**中断触发条件**：
- 当PA1检测到上升沿时
- 自动将CNT值存入CCR2寄存器
- 设置SR寄存器的CC2IF位
- 如果DIER的CC2IE位已使能，则产生中断请求
- CPU跳转到TIM2_IRQHandler执行

---

#### 3.5 HAL库中断处理 - HAL_TIM_IRQHandler()

**位置**：`Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_tim.c` 第3822行

```c
void HAL_TIM_IRQHandler(TIM_HandleTypeDef *htim)
{
  uint32_t itsource = htim->Instance->DIER;  // ← 读取中断使能寄存器
  // DIER = DMA/Interrupt Enable Register
  // 位0：UIE - 更新中断使能
  // 位1：CC1IE - 捕获/比较1中断使能
  // 位2：CC2IE - 捕获/比较2中断使能 ← 本项目使用
  // 位3：CC3IE - 捕获/比较3中断使能
  // 位4：CC4IE - 捕获/比较4中断使能
  // 位5：COMIE - COM中断使能
  // 位6：TIE - 触发中断使能
  // 位7：BIE - 布置中断使能
  // 位8-15：DMA使能位
  
  uint32_t itflag   = htim->Instance->SR;  // ← 读取中断标志寄存器
  // SR = Status Register
  // 位0：UIF - 更新中断标志
  // 位1：CC1IF - 捕获/比较1中断标志
  // 位2：CC2IF - 捕获/比较2中断标志 ← 本项目使用
  // 位3：CC3IF - 捕获/比较3中断标志
  // 位4：CC4IF - 捕获/比较4中断标志
  // 位5：COMIF - COM中断标志
  // 位6：TIF - 触发中断标志
  // 位7：BIF - 布置中断标志
  // 位8-15：DMA标志

  // 1. 检查捕获/比较2事件
  if ((itflag & (TIM_FLAG_CC2)) == (TIM_FLAG_CC2))  // ← 检查CC2IF标志
  {
    // 2. 检查中断是否使能
    if ((itsource & (TIM_IT_CC2)) == (TIM_IT_CC2))
    {
      __HAL_TIM_CLEAR_FLAG(htim, TIM_FLAG_CC2);  // ← 清除中断标志
      // 写0清除：TIM2->SR &= ~TIM_SR_CC2IF
      // 注意：SR寄存器的某些位需要写1清除，某些位需要写0清除
      // CC2IF需要写0清除（或写1清除，具体看手册）
      
      htim->Channel = HAL_TIM_ACTIVE_CHANNEL_2;  // ← 设置当前通道为CH2
      
      // 3. 判断是输入捕获还是输出比较
      if ((htim->Instance->CCMR1 & TIM_CCMR1_CC2S) != 0x00U)
      {
        // 4. 输入捕获事件
        // CCMR1 = Capture/Compare Mode Register 1
        // CC2S[1:0] = CCMR1[3:2] = 01: IC2映射到TI2
        // != 0x00U 表示配置为输入捕获模式
        
#if (USE_HAL_TIM_REGISTER_CALLBACKS == 1)
        htim->IC_CaptureCallback(htim);  // 使用注册的回调函数
#else
        HAL_TIM_IC_CaptureCallback(htim);  // 调用默认回调函数
#endif
      }
      // 5. 输出比较事件（本项目不涉及）
      else
      {
#if (USE_HAL_TIM_REGISTER_CALLBACKS == 1)
        htim->OC_DelayElapsedCallback(htim);
        htim->PWM_PulseFinishedCallback(htim);
#else
        HAL_TIM_OC_DelayElapsedCallback(htim);
        HAL_TIM_PWM_PulseFinishedCallback(htim);
#endif
      }
      
      htim->Channel = HAL_TIM_ACTIVE_CHANNEL_CLEARED;  // ← 清除通道标志
    }
  }
  
  // 其他通道的处理（CC1, CC3, CC4）...
}
```

**执行流程**：

```
1. 读取DIER和SR寄存器
   ↓
2. 检查CC2IF标志是否置位
   ↓
3. 检查CC2IE中断是否使能
   ↓
4. 清除CC2IF标志
   ↓
5. 设置当前通道为CH2
   ↓
6. 判断CC2S位，确认是输入捕获模式
   ↓
7. 调用HAL_TIM_IC_CaptureCallback()回调函数
   ↓
8. 清除通道标志
```

---

#### 3.6 用户回调函数 - HAL_TIM_IC_CaptureCallback()

**位置**：`main.c` 第165-247行

```c
// TIM2输入捕获回调函数 - 使用CH2(PA1)捕获FG信号
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)  // ← 用户回调函数
{
  // 1. 判断是否是TIM2且通道2
  if (htim->Instance == TIM2 && htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)
  {
    static uint32_t callback_count = 0;  // ← 回调函数调用次数（静态变量）
    callback_count++;
    
    // 2. 读取捕获值
    uint32_t captured_value = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2);
    // 函数实现：
    // uint32_t tmp = htim->Instance->CCR2;  // 读取捕获寄存器
    // return tmp;
    
    // 3. Debug打印（前几次）
    if (fg_debug_enabled && callback_count <= 5)
    {
      UART_Printf("[FG DEBUG] Callback #%lu: TIM2_CH2, Value=%lu\r\n",
                  callback_count, captured_value);
    }
    
    // 4. 第一次捕获，只记录时间
    if (fg_capture_count == 0)  // ← 第一次捕获
    {
      fg_last_time = captured_value;  // 记录第一次捕获值
      fg_current_time = captured_value;
      fg_capture_count++;
      last_fg_update_time = HAL_GetTick();  // 记录当前时间戳
      
      if (fg_debug_enabled)
      {
        UART_Printf("[FG DEBUG] First capture: %lu\r\n", captured_value);
      }
      return;  // 第一次捕获不计算频率
    }
    
    // 5. 后续捕获，计算频率
    fg_last_time = fg_current_time;  // 保存上一次捕获值
    fg_current_time = captured_value;  // 保存当前捕获值
    
    if (fg_current_time > fg_last_time)  // 正常情况（未溢出）
    {
      // 6. 计算周期（计数差值）
      uint32_t fg_period = fg_current_time - fg_last_time;
      
      // 7. 计算频率
      // 计数频率 = 10kHz (周期 = 0.1ms)
      // fg_period = 两次捕获的计数差
      // fg_period = 100 → 周期 = 100 × 0.1ms = 10ms → 频率 = 100Hz
      // fg_period = 50 → 周期 = 50 × 0.1ms = 5ms → 频率 = 200Hz
      fg_frequency = 10000.0f / (float)fg_period;  // 频率 = 10kHz / 计数差
      
      // 8. 更新时间戳
      last_fg_update_time = HAL_GetTick();
    }
    else  // 溢出情况（CNT重载）
    {
      // 说明：定时器计数范围0-65535，如果两次捕获之间发生了溢出
      // 需要加上溢出的计数值
      uint32_t overflow_count = 1;  // 简化处理，假设溢出1次
      uint32_t fg_period = (65536 * overflow_count) - fg_last_time + fg_current_time;
      fg_frequency = 10000.0f / (float)fg_period;
    }
    
    fg_capture_count++;  // 捕获计数+1
    
    // 9. Debug打印（前10次或每100次）
    if (fg_debug_enabled && (fg_capture_count <= 10 || fg_capture_count % 100 == 0))
    {
      UART_Printf("[FG DEBUG] Capture #%lu: curr=%lu, last=%lu, period=%lu, freq=%.2f Hz\r\n",
                  fg_capture_count, fg_current_time, fg_last_time, 
                  fg_current_time - fg_last_time, fg_frequency);
    }
  }
}
```

**回调函数详解**：

1. **htim->Instance == TIM2**
   - 判断是否是TIM2定时器
   - 确保是正确的定时器

2. **htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2**
   - 判断是否是通道2
   - 确保是正确的通道

3. **HAL_TIM_ReadCapturedValue()**
   - 读取捕获寄存器CCR2的值
   - CCR2 = 捕获发生时CNT的值
   - 返回值 = 当前捕获的计数值

4. **第一次捕获处理**
   - `fg_capture_count == 0`表示第一次捕获
   - 只记录初始值，不计算频率
   - 因为需要两次捕获才能计算周期

5. **后续捕获处理**
   - 计算计数差值：`fg_period = fg_current_time - fg_last_time`
   - 计算频率：`fg_frequency = 10000.0f / (float)fg_period`
   - 计数频率 = 10kHz，所以频率 = 10kHz / 计数差

6. **频率计算原理**
   ```
   计数频率 = 10kHz (周期 = 0.1ms)
   
   例1：FG信号1kHz（600RPM）
   - 每个周期1ms = 10个计数
   - fg_period = 10
   - fg_frequency = 10000 / 10 = 1000Hz ✓
   
   例2：FG信号2kHz（1200RPM）
   - 每个周期0.5ms = 5个计数
   - fg_period = 5
   - fg_frequency = 10000 / 5 = 2000Hz ✓
   ```

---

### 4. 寄存器映射

#### 4.1 TIM2关键寄存器

```
TIM2基地址：0x4000 0000

控制寄存器1 (CR1)：
位0: CEN - 计数器使能
位1: UDIS - 禁止更新
位2: URS - 仅使用更新请求
位3: OPM - 单次模式
位4: DIR - 方向（0:向上，1:向下）
位5: CMS - 中央对齐模式
位6: ARPE - 自动重载预加载使能
位7: CKD - 时钟分频

自动重载寄存器 (ARR)：
地址：0x4000 002C
值：65535 (0xFFFF)
范围：0-65535 (16位)

预分频寄存器 (PSC)：
地址：0x4000 0028
值：6399 (0x191F)
分频系数：PSC + 1 = 6400

计数器 (CNT)：
地址：0x4000 0024
值：当前计数值
范围：0-65535

捕获/比较寄存器2 (CCR2)：
地址：0x4000 0038
值：捕获发生时CNT的值
只读（捕获模式）

捕获/比较使能寄存器 (CCER)：
地址：0x4000 0030
位0: CC2E - 捕获/比较2输出使能
位1: CC2P - 捕获/比较2极性（0:上升沿，1:下降沿）
位2: CC2NE - 捕获/比较2互补输出使能
位3: CC2NP - 捕获/比较2互补极性

捕获/比较模式寄存器1 (CCMR1)：
地址：0x4000 0018
位7-0: OC2M - 输出比较模式
位15-8: IC2F - 输入捕获滤波器
位13-12: IC2PSC - 输入捕获预分频器
位9-8: CC2S - 输入捕获选择

DMA/中断使能寄存器 (DIER)：
地址：0x4000 000C
位0: UIE - 更新中断使能
位1: CC1IE - 捕获/比较1中断使能
位2: CC2IE - 捕获/比较2中断使能 ← 本项目使用
位3: CC3IE - 捕获/比较3中断使能
位4: CC4IE - 捕获/比较4中断使能

状态寄存器 (SR)：
地址：0x4000 0010
位0: UIF - 更新中断标志
位1: CC1IF - 捕获/比较1中断标志
位2: CC2IF - 捕获/比较2中断标志 ← 本项目使用
位3: CC3IF - 捕获/比较3中断标志
位4: CC4IF - 捕获/比较4中断标志
```

#### 4.2 关键寄存器配置

```
初始化后：

CR1 = 0x00000001  (CEN = 1, 计数器使能)
ARR = 0x0000FFFF  (65535)
PSC = 0x0000191F  (6399)
CNT = 0x00000000  (初始值)
CCR2 = 0x00000000  (初始值)
CCER = 0x00000001  (CC2E = 1, CC2P = 0)
CCMR1 = 0x00000100  (CC2S = 01, IC2F = 1111)
DIER = 0x00000004  (CC2IE = 1)
SR = 0x00000000  (所有标志清除)
```

---

### 5. 完整执行流程图

```
系统上电
    ↓
main() 函数开始
    ↓
MX_TIM2_Init()  // tim.c 第61行
    ├─ htim2.Instance = TIM2
    ├─ htim2.Init.Prescaler = 6399  (10kHz计数频率)
    ├─ htim2.Init.Period = 65535
    ├─ HAL_TIM_IC_Init(&htim2)
    │   ├─ 设置PSC、ARR等寄存器
    │   └─ 调用HAL_TIM_IC_MspInit()
    └─ HAL_TIM_IC_MspInit(&htim2)
        ├─ __HAL_RCC_TIM2_CLK_ENABLE()  (使能TIM2时钟)
        ├─ __HAL_RCC_GPIOA_CLK_ENABLE()  (使能GPIOA时钟)
        ├─ HAL_GPIO_Init(GPIOA, &GPIO_InitStruct)  (配置PA1为输入)
        ├─ HAL_NVIC_SetPriority(TIM2_IRQn, 2, 0)  (设置中断优先级)
        └─ HAL_NVIC_EnableIRQ(TIM2_IRQn)  (使能TIM2中断)
    ↓
main() 继续执行其他初始化
    ↓
HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_2)  // main.c 第749行
    ├─ __HAL_TIM_ENABLE_IT(htim, TIM_IT_CC2)  (使能CC2中断)
    ├─ TIM_CCxChannelCmd()  (使能CC2通道)
    └─ __HAL_TIM_ENABLE(htim)  (使能定时器)
        └─ TIM2->CR1 |= TIM_CR1_CEN  (CNT开始计数)
    ↓
定时器开始计数
    ├─ CNT从0开始向上计数
    ├─ 计数频率：10kHz (每100μs加1)
    └─ 计数范围：0-65535
    ↓
FG信号输入到PA1
    ├─ 上升沿触发捕获
    └─ 自动将CNT值存入CCR2
    ↓
设置SR寄存器的CC2IF标志
    ↓
如果DIER的CC2IE已使能
    ↓
产生TIM2中断请求
    ↓
CPU跳转到TIM2_IRQHandler
    ↓
HAL_TIM_IRQHandler(&htim2)
    ├─ 读取DIER和SR寄存器
    ├─ 检查CC2IF标志
    ├─ 清除CC2IF标志
    ├─ 设置当前通道为CH2
    ├─ 判断是输入捕获模式
    └─ 调用HAL_TIM_IC_CaptureCallback(&htim2)
    ↓
HAL_TIM_IC_CaptureCallback()  // main.c 第165行
    ├─ 读取CCR2值：captured_value
    ├─ 第一次捕获：只记录
    └─ 后续捕获：计算频率
        ├─ fg_period = fg_current_time - fg_last_time
        ├─ fg_frequency = 10000.0f / (float)fg_period
        └─ 打印调试信息
    ↓
返回主程序
    ↓
主循环继续执行
```

---

### 6. 关键代码文件汇总

| 文件 | 函数 | 作用 |
|------|------|------|
| `tim.c` | `MX_TIM2_Init()` | 配置TIM2输入捕获参数 |
| `tim.c` | `HAL_TIM_IC_MspInit()` | 配置GPIO和NVIC |
| `main.c` | `HAL_TIM_IC_Start_IT()` | 启动输入捕获中断 |
| `stm32f1xx_it.c` | `TIM2_IRQHandler()` | 中断服务程序入口 |
| `stm32f1xx_hal_tim.c` | `HAL_TIM_IRQHandler()` | HAL库中断处理 |
| `main.c` | `HAL_TIM_IC_CaptureCallback()` | 用户回调函数 |

---

### 7. 调试技巧

#### 7.1 打印调试信息

```c
// 启用FG捕获Debug
fg_debug_enabled = 1;

// 查看捕获值
UART_Printf("Captured Value: %lu\r\n", captured_value);

// 查看频率
UART_Printf("FG Frequency: %.2f Hz\r\n", fg_frequency);

// 查看计数差
UART_Printf("Period: %lu\r\n", fg_current_time - fg_last_time);
```

#### 7.2 常见问题排查

| 问题现象 | 可能原因 | 检查方法 |
|----------|----------|----------|
| 收不到FG信号 | PA1引脚未配置 | 检查GPIO配置 |
| 捕获值不变 | FG信号频率太低 | 用示波器测量PA1 |
| 频率计算错误 | 计数频率不正确 | 检查PSC和ARR配置 |
| 中断不触发 | 中断未使能 | 检查DIER和NVIC配置 |
| 捕获值溢出 | FG信号频率太高 | 增加PSC或使用双边沿捕获 |

---

## 调试技巧

### 1. 使用串口打印调试

```c
// 打印字符串
UART_SendString("[INFO] System initialized\r\n");

// 打印格式化数据
UART_Printf("[DEBUG] PWM duty = %u%%\r\n", duty);
UART_Printf("[DEBUG] ADC value = %u, voltage = %.2fV\r\n", adc_value, voltage);
```

### 2. 使用示波器测量

| 信号 | 引脚 | 频率 | 占空比 | 用途 |
|------|------|------|--------|------|
| PWM_PUMP | PC6 | 100Hz | 可调 | 水泵控制 |
| PWM_FAN1 | PC7 | 100Hz | 可调 | 风扇1控制 |
| PWM_FAN2 | PC8 | 100Hz | 可调 | 风扇2控制 |
| PWM_FAN3 | PC9 | 100Hz | 可调 | 风扇3控制 |
| MCO1 | PA8 | 72MHz | - | 系统时钟输出 |

### 3. 使用逻辑分析仪

- **捕获I2C数据**：SCL（PB10）、SDA（PB11）
- **捕获USART数据**：TX（PA9/PA2）、RX（PA10/PA3）
- **捕获FG信号**：PA1

### 4. 常用调试命令

| 命令 | 功能 | 示例 |
|------|------|------|
| `NTC_START` | 开始NTC温度读取 | 发送 `NTC_START` |
| `NTC_STOP` | 停止NTC温度读取 | 发送 `NTC_STOP` |
| `PWM50` | 设置PWM占空比50% | 发送 `PWM50` |
| `S1 10.5` | 设置TEC温度10.5℃ | 发送 `S1 10.5` |

---

## 常见问题

### Q1: 编译错误 "undefined reference to HAL_xxx"

**原因**：缺少HAL库源文件或未正确包含

**解决**：
1. 检查 `Drivers/STM32F1xx_HAL_Driver/Src/` 目录下是否有对应文件
2. 在Keil中添加缺失的源文件到项目
3. 或使用STM32CubeMX重新生成代码

### Q2: 串口助手收不到数据

**检查步骤**：
1. 确认波特率设置正确（USART1: 115200，USART2: 9600）
2. 确认TX/RX引脚连接正确（TX接RX，RX接TX）
3. 检查串口助手是否启用了换行符（\r\n）
4. 使用示波器测量TX引脚是否有波形

### Q3: PWM没有输出

**检查步骤**：
1. 确认TIMx时钟已使能（`__HAL_RCC_TIMx_CLK_ENABLE()`）
2. 确认GPIO模式设置为复用推挽（`GPIO_MODE_AF_PP`）
3. 确认PWM通道已启动（`HAL_TIM_PWM_Start()`）
4. 使用示波器测量引脚

### Q4: I2C通信失败

**检查步骤**：
1. 确认SCL/SDA引脚配置为开漏输出（`GPIO_MODE_AF_OD`）
2. 确认上拉电阻已焊接（通常4.7kΩ）
3. 检查I2C地址是否正确（TLA2528: 0x84）
4. 使用逻辑分析仪捕获I2C波形

### Q5: 风扇转速为0

**检查步骤**：
1. 确认FG信号已连接到PA1（TIM2_CH2）
2. 确认TIM2输入捕获已启动（`HAL_TIM_IC_Start_IT()`）
3. 检查风扇是否正常工作（用示波器测量FG信号）
4. 确认FG频率计算公式正确（RPM = Freq * 60 / 2）

---

## 学习资源

### 官方文档
- **STM32F103参考手册**：https://www.st.com/resource/en/reference_manual/cd00171190-stm32f101xx-stm32f102xx-stm32f103xx-advanced-arm-based-32-bit-mcus-stmicroelectronics.pdf
- **HAL库用户手册**：https://www.st.com/resource/en/user_manual/dm00105873-stm32cube-mcu-families-stmicroelectronics.pdf
- **TLA2528数据手册**：https://www.ti.com/lit/ds/symlink/tla2528.pdf

### 在线资源
- **STM32CubeMX教程**：https://www.st.com/content/st_com/en/products/development-tools/product-software/stm32cubemx.html
- **Keil MDK教程**：https://www.keil.com/support/man/docs/uv4/
- **STM32论坛**：https://community.st.com/

---

## 总结

本项目是一个典型的嵌入式系统，涵盖了STM32的多个外设模块：

1. **GPIO**：控制LED、MUX等
2. **USART**：串口通信
3. **TIM**：PWM输出、输入捕获
4. **I2C**：与ADC通信
5. **中断**：实时响应

**学习建议**：
1. 先理解每个模块的独立功能
2. 再理解模块之间的协同工作
3. 最后理解整体系统的运行逻辑

**祝学习愉快！** 🎉

---

## 新手如何查找和使用HAL库函数

### 1. 问题：新手怎么知道要用到哪些函数？

#### 1.1 理解外设功能

**第一步：明确需求**

```
需求：测量风扇FG信号频率
├─ 什么是FG信号？
│   └─ 风扇转速反馈信号，通常是方波
├─ 需要测量什么？
│   └─ 频率（每秒脉冲数）
└─ 需要什么硬件？
    └─ 定时器输入捕获功能
```

**第二步：查找对应外设**

```
测量频率 → 需要定时器 → STM32有多个定时器
├─ TIM1: 高级控制定时器（复杂）
├─ TIM2: 通用定时器（适合本项目）
├─ TIM3: 通用定时器（已用于PWM）
├─ TIM4: 通用定时器
└─ TIM8: 高级控制定时器（复杂）

选择：TIM2（通用定时器，功能简单）
```

**第三步：查找对应功能**

```
TIM2功能：
├─ 定时器模式（TIM_BASE）
│   └─ HAL_TIM_Base_Init() / HAL_TIM_Base_Start()
├─ PWM输出模式（TIM_PWM）
│   └─ HAL_TIM_PWM_Init() / HAL_TIM_PWM_Start()
├─ 输入捕获模式（TIM_IC） ← 需要这个
│   └─ HAL_TIM_IC_Init() / HAL_TIM_IC_Start()
│   └─ HAL_TIM_IC_Start_IT() ← 需要中断方式
└─ 输出比较模式（TIM_OC）
    └─ HAL_TIM_OC_Init() / HAL_TIM_OC_Start()
```

#### 1.2 查找函数的方法

**方法1：查看头文件**

**文件**：`Drivers/STM32F1xx_HAL_Driver/Inc/stm32f1xx_hal_tim.h`

```c
// 输入捕获相关函数
HAL_StatusTypeDef HAL_TIM_IC_Init(TIM_HandleTypeDef *htim);
HAL_StatusTypeDef HAL_TIM_IC_Start(TIM_HandleTypeDef *htim, uint32_t Channel);
HAL_StatusTypeDef HAL_TIM_IC_Start_IT(TIM_HandleTypeDef *htim, uint32_t Channel);  // ← 需要这个
HAL_StatusTypeDef HAL_TIM_IC_Start_DMA(TIM_HandleTypeDef *htim, uint32_t Channel);
void HAL_TIM_IC_IRQHandler(TIM_HandleTypeDef *htim);
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim);  // ← 需要这个
```

**方法2：查看.c文件中的函数**

**文件**：`Src/tim.c`

```c
// 初始化函数
void MX_TIM2_Init(void)
{
  // ...
  if (HAL_TIM_IC_Init(&htim2) != HAL_OK)  // ← 初始化
  {
    Error_Handler();
  }
  // ...
  if (HAL_TIM_IC_ConfigChannel(&htim2, &sConfigIC, TIM_CHANNEL_2) != HAL_OK)  // ← 配置通道
  {
    Error_Handler();
  }
}

// MSP初始化
void HAL_TIM_IC_MspInit(TIM_HandleTypeDef* tim_icHandle)
{
  // ...
}

// 启动函数在main.c中调用
// HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_2);
```

**方法3：查看回调函数**

**文件**：`Src/main.c`

```c
// 用户回调函数
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM2 && htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)
  {
    // 处理捕获数据
  }
}
```

#### 1.3 函数选择指南

**问题1：用HAL_TIM_IC_Start还是HAL_TIM_IC_Start_IT？**

```
HAL_TIM_IC_Start()  // 轮询方式
├─ 优点：简单，不需要中断
├─ 缺点：CPU一直等待，效率低
└─ 适用：简单测量，不关心实时性

HAL_TIM_IC_Start_IT()  // 中断方式 ← 推荐
├─ 优点：CPU可以做其他事，实时性好
├─ 缺点：需要配置中断，代码复杂
└─ 适用：实时系统，需要快速响应

选择：HAL_TIM_IC_Start_IT() ← 本项目使用
```

**问题2：用哪个通道？**

```
TIM2有4个通道：
├─ TIM_CHANNEL_1 (PA0) - 未使用
├─ TIM_CHANNEL_2 (PA1) - FG信号输入 ← 选择这个
├─ TIM_CHANNEL_3 (PA2) - USART2_TX，不能用
└─ TIM_CHANNEL_4 (PA3) - USART2_RX，不能用

选择：TIM_CHANNEL_2
```

**问题3：需要配置哪些参数？**

```
查看HAL_TIM_IC_ConfigChannel()函数参数：

HAL_StatusTypeDef HAL_TIM_IC_ConfigChannel(
    TIM_HandleTypeDef *htim,        // 定时器句柄
    TIM_IC_InitTypeDef *sConfigIC,  // 配置结构体
    uint32_t Channel)               // 通道号
{
  // ...
}

TIM_IC_InitTypeDef结构体定义：
├─ ICPolarity: 捕获边沿
│   ├─ TIM_INPUTCHANNELPOLARITY_RISING: 上升沿
│   ├─ TIM_INPUTCHANNELPOLARITY_FALLING: 下降沿
│   └─ TIM_INPUTCHANNELPOLARITY_BOTHEDGE: 双边沿
├─ ICSelection: 输入选择
│   ├─ TIM_ICSELECTION_DIRECTTI: 直接输入
│   ├─ TIM_ICSELECTION_INDIRECTTI: 间接输入
│   └─ TIM_ICSELECTION_TRC: 触发输入
├─ ICPrescaler: 预分频
│   ├─ TIM_ICPSC_DIV1: 每个边沿都捕获
│   ├─ TIM_ICPSC_DIV2: 每2个边沿捕获一次
│   ├─ TIM_ICPSC_DIV4: 每4个边沿捕获一次
│   └─ TIM_ICPSC_DIV8: 每8个边沿捕获一次
└─ ICFilter: 数字滤波器
    └─ 0-15: 滤波长度（0=不滤波，15=最强滤波）

本项目选择：
├─ ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING  (上升沿)
├─ ICSelection = TIM_ICSELECTION_DIRECTTI        (直接输入)
├─ ICPrescaler = TIM_ICPSC_DIV1                  (不分频)
└─ ICFilter = 15                                 (滤波长度15)
```

---

### 2. 问题：从哪里找对应的函数定义？

#### 2.1 函数定义位置

```
HAL库函数定义位置：

 Drivers/
 └─ STM32F1xx_HAL_Driver/
     ├─ Inc/                    ← 头文件（函数声明）
     │   ├─ stm32f1xx_hal_tim.h     ← 定时器函数声明
     │   └─ stm32f1xx_hal_uart.h    ← 串口函数声明
     │
     └─ Src/                    ← 源文件（函数实现）
         ├─ stm32f1xx_hal_tim.c     ← 定时器函数实现
         └─ stm32f1xx_hal_uart.c    ← 串口函数实现

项目代码位置：

 Src/
 ├─ tim.c                     ← 项目定时器代码
 │   ├─ MX_TIM2_Init()          ← 项目初始化函数
 │   └─ HAL_TIM_IC_MspInit()    ← MSP初始化
 │
 ├─ main.c                    ← 主程序
 │   └─ HAL_TIM_IC_CaptureCallback()  ← 用户回调函数
 │
 └─ stm32f1xx_it.c            ← 中断服务程序
     └─ TIM2_IRQHandler()       ← 中断服务程序

函数调用链：
main.c (用户代码)
    ↓
HAL_TIM_IC_Start_IT()  (HAL库函数)
    ↓
stm32f1xx_hal_tim.c (HAL库实现)
    ↓
硬件寄存器配置
```

#### 2.2 如何查找函数

**步骤1：确定外设**

```
需求：输入捕获
→ 外设：TIM (定时器)
→ 文件：stm32f1xx_hal_tim.h / .c
```

**步骤2：查看头文件**

**文件**：`Drivers/STM32F1xx_HAL_Driver/Inc/stm32f1xx_hal_tim.h`

```c
// 查找输入捕获相关函数
// 搜索 "Input Capture" 或 "IC"

// 初始化函数
HAL_StatusTypeDef HAL_TIM_IC_Init(TIM_HandleTypeDef *htim);

// 配置通道函数
HAL_StatusTypeDef HAL_TIM_IC_ConfigChannel(TIM_HandleTypeDef *htim,
                                            TIM_IC_InitTypeDef *sConfigIC,
                                            uint32_t Channel);

// 启动函数
HAL_StatusTypeDef HAL_TIM_IC_Start(TIM_HandleTypeDef *htim, uint32_t Channel);
HAL_StatusTypeDef HAL_TIM_IC_Start_IT(TIM_HandleTypeDef *htim, uint32_t Channel);  // ← 需要
HAL_StatusTypeDef HAL_TIM_IC_Start_DMA(TIM_HandleTypeDef *htim, uint32_t Channel);

// 中断处理函数
void HAL_TIM_IC_IRQHandler(TIM_HandleTypeDef *htim);

// 回调函数
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim);  // ← 需要
```

**步骤3：查看函数原型**

```c
// HAL_TIM_IC_Start_IT() 函数原型
HAL_StatusTypeDef HAL_TIM_IC_Start_IT(TIM_HandleTypeDef *htim, uint32_t Channel);

// 参数说明：
// htim: TIM_HandleTypeDef *htim  - 定时器句柄（包含配置信息）
// Channel: uint32_t Channel      - 通道号（TIM_CHANNEL_1/2/3/4）

// 返回值：
// HAL_OK: 成功
// HAL_ERROR: 错误
// HAL_BUSY: 忙
// HAL_TIMEOUT: 超时
```

**步骤4：查看函数实现**

**文件**：`Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_tim.c`

```c
HAL_StatusTypeDef HAL_TIM_IC_Start_IT(TIM_HandleTypeDef *htim, uint32_t Channel)
{
  // 1. 检查参数
  // 2. 设置通道状态
  // 3. 使能中断
  // 4. 使能通道
  // 5. 使能定时器
  // 6. 返回状态
}
```

#### 2.3 如何查找结构体定义

**步骤1：查看头文件**

```c
// TIM_IC_InitTypeDef结构体定义
typedef struct
{
  uint32_t ICPolarity;   // 捕获边沿
  uint32_t ICSelection;  // 输入选择
  uint32_t ICPrescaler;  // 预分频
  uint32_t ICFilter;     // 滤波器
} TIM_IC_InitTypeDef;
```

**步骤2：查看可用值**

```c
// ICPolarity可用值
#define TIM_INPUTCHANNELPOLARITY_RISING      0x00000000U  // 上升沿
#define TIM_INPUTCHANNELPOLARITY_FALLING     TIM_CCER_CC1P  // 下降沿
#define TIM_INPUTCHANNELPOLARITY_BOTHEDGE   (TIM_CCER_CC1P | TIM_CCER_CC1NP)  // 双边沿

// ICSelection可用值
#define TIM_ICSELECTION_DIRECTTI           0x00000000U  // 直接输入
#define TIM_ICSELECTION_INDIRECTTI         TIM_CCMR1_CC1S_0  // 间接输入
#define TIM_ICSELECTION_TRC                TIM_CCMR1_CC1S_1  // 触发输入

// ICPrescaler可用值
#define TIM_ICPSC_DIV1                     0x00000000U  // 不分频
#define TIM_ICPSC_DIV2                     TIM_CCMR1_IC1PSC_0  // 2分频
#define TIM_ICPSC_DIV4                     TIM_CCMR1_IC1PSC_1  // 4分频
#define TIM_ICPSC_DIV8                     TIM_CCMR1_IC1PSC  // 8分频
```

---

### 3. 问题：如何选择并调用函数？

#### 3.1 完整调用流程

**步骤1：初始化定时器**

```c
// 1. 定义句柄（全局变量）
TIM_HandleTypeDef htim2;

// 2. 调用初始化函数
MX_TIM2_Init();

// 3. 初始化函数内部调用
HAL_TIM_IC_Init(&htim2);  // 初始化输入捕获模式
```

**步骤2：配置通道参数**

```c
// 1. 定义配置结构体
TIM_IC_InitTypeDef sConfigIC = {0};

// 2. 配置参数
sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;  // 上升沿捕获
sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;        // 直接输入
sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;                  // 不分频
sConfigIC.ICFilter = 15;                                 // 滤波长度15

// 3. 调用配置函数
HAL_TIM_IC_ConfigChannel(&htim2, &sConfigIC, TIM_CHANNEL_2);
```

**步骤3：启动输入捕获**

```c
// 调用启动函数（中断方式）
if (HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_2) != HAL_OK)
{
  // 启动失败，处理错误
  Error_Handler();
}
else
{
  // 启动成功
  UART_SendString("[INFO] TIM2 IC started\r\n");
}
```

**步骤4：实现回调函数**

```c
// 在main.c中实现回调函数
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
  // 1. 判断是否是TIM2且通道2
  if (htim->Instance == TIM2 && htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)
  {
    // 2. 读取捕获值
    uint32_t captured_value = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2);
    
    // 3. 处理捕获数据
    // ... (计算频率、打印调试信息等)
  }
}
```

#### 3.2 函数选择决策树

```
需要输入捕获功能？
├─ 是
│   ├─ 选择定时器？
│   │   ├─ TIM1/TIM8 (高级控制) → 复杂，不推荐
│   │   └─ TIM2/TIM3/TIM4 (通用) → 简单，推荐
│   │       └─ 选择TIM2
│   │
│   ├─ 选择工作方式？
│   │   ├─ 轮询方式 (HAL_TIM_IC_Start)
│   │   │   ├─ 优点：简单
│   │   │   └─ 缺点：CPU一直等待
│   │   └─ 中断方式 (HAL_TIM_IC_Start_IT) ← 推荐
│   │       ├─ 优点：CPU可以做其他事
│   │       └─ 缺点：需要配置中断
│   │
│   ├─ 选择通道？
│   │   ├─ TIM_CHANNEL_1 (PA0)
│   │   ├─ TIM_CHANNEL_2 (PA1) ← 本项目使用
│   │   ├─ TIM_CHANNEL_3 (PA2)
│   │   └─ TIM_CHANNEL_4 (PA3)
│   │
│   └─ 需要回调函数？
       ├─ 是（中断方式必须）
       │   └─ 实现 HAL_TIM_IC_CaptureCallback()
       └─ 否（轮询方式）
```

#### 3.3 常用函数列表

**输入捕获相关函数**：

```c
// 初始化
HAL_TIM_IC_Init()                    // 初始化输入捕获模式
HAL_TIM_IC_MspInit()                 // MSP初始化（自动调用）
HAL_TIM_IC_ConfigChannel()           // 配置通道参数

// 启动
HAL_TIM_IC_Start()                   // 轮询方式启动
HAL_TIM_IC_Start_IT()                // 中断方式启动 ← 推荐
HAL_TIM_IC_Start_DMA()               // DMA方式启动

// 停止
HAL_TIM_IC_Stop()                    // 停止轮询方式
HAL_TIM_IC_Stop_IT()                 // 停止中断方式
HAL_TIM_IC_Stop_DMA()                // 停止DMA方式

// 读取
uint32_t HAL_TIM_ReadCapturedValue() // 读取捕获值

// 中断处理
void HAL_TIM_IC_IRQHandler()         // 中断处理函数（自动调用）

// 回调函数（需要用户实现）
void HAL_TIM_IC_CaptureCallback()    // 捕获回调函数 ← 需要实现
```

**串口相关函数**：

```c
// 初始化
HAL_UART_Init()                      // 初始化串口
HAL_UART_MspInit()                   // MSP初始化

// 发送
HAL_UART_Transmit()                  // 轮询发送
HAL_UART_Transmit_IT()               // 中断发送
HAL_UART_Transmit_DMA()              // DMA发送

// 接收
HAL_UART_Receive()                   // 轮询接收
HAL_UART_Receive_IT()                // 中断接收 ← 推荐
HAL_UART_Receive_DMA()               // DMA接收

// 回调函数
void HAL_UART_RxCpltCallback()       // 接收完成回调
```

**PWM相关函数**：

```c
// 初始化
HAL_TIM_PWM_Init()                   // 初始化PWM模式
HAL_TIM_PWM_ConfigChannel()          // 配置通道
HAL_TIM_PWM_Start()                  // 启动PWM
HAL_TIM_PWM_Start_IT()               // 中断方式启动

// 设置占空比
__HAL_TIM_SET_COMPARE()              // 宏，直接设置CCR寄存器
```

#### 3.4 函数调用示例

**完整示例：配置TIM2输入捕获**

```c
// 1. 全局变量定义
TIM_HandleTypeDef htim2;  // 定时器句柄

// 2. 初始化函数（在MX_TIM2_Init中）
void MX_TIM2_Init(void)
{
  // 配置定时器参数
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 6399;  // 10kHz计数频率
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 65535;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  
  // 初始化输入捕获模式
  if (HAL_TIM_IC_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  
  // 配置通道2
  TIM_IC_InitTypeDef sConfigIC = {0};
  sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
  sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
  sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
  sConfigIC.ICFilter = 15;
  
  if (HAL_TIM_IC_ConfigChannel(&htim2, &sConfigIC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
}

// 3. 启动函数（在main中）
HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_2);

// 4. 回调函数（在main.c中）
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM2 && htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)
  {
    uint32_t captured_value = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2);
    // 处理捕获数据...
  }
}
```

---

### 4. 实践步骤：从零开始写FG捕获代码

#### 4.1 步骤1：确定需求

```
需求：测量风扇FG信号频率
├─ FG信号：方波，频率与转速成正比
├─ 测量：频率（Hz）
└─ 输出：串口打印频率
```

#### 4.2 步骤2：选择外设

```
测量频率 → 定时器输入捕获
├─ 选择TIM2（通用定时器）
└─ 选择通道2（PA1，FG信号引脚）
```

#### 4.3 步骤3：查找函数

**文件**：`Drivers/STM32F1xx_HAL_Driver/Inc/stm32f1xx_hal_tim.h`

```c
// 需要的函数
HAL_TIM_IC_Init()                    // 初始化
HAL_TIM_IC_ConfigChannel()           // 配置
HAL_TIM_IC_Start_IT()                // 启动中断
HAL_TIM_IC_CaptureCallback()         // 回调函数
```

#### 4.4 步骤4：编写代码

**步骤4.1：定义句柄**

```c
// 在main.h或main.c顶部
extern TIM_HandleTypeDef htim2;
```

**步骤4.2：配置定时器**

```c
// 在MX_TIM2_Init()中
htim2.Instance = TIM2;
htim2.Init.Prescaler = 6399;  // 10kHz
htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
htim2.Init.Period = 65535;
HAL_TIM_IC_Init(&htim2);
```

**步骤4.3：配置通道**

```c
TIM_IC_InitTypeDef sConfigIC = {0};
sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
sConfigIC.ICFilter = 15;
HAL_TIM_IC_ConfigChannel(&htim2, &sConfigIC, TIM_CHANNEL_2);
```

**步骤4.4：启动**

```c
HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_2);
```

**步骤4.5：实现回调**

```c
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM2 && htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)
  {
    uint32_t captured_value = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2);
    // 处理数据...
  }
}
```

#### 4.5 步骤5：调试验证

```c
// 启用Debug
fg_debug_enabled = 1;

// 查看捕获值
UART_Printf("Captured: %lu\r\n", captured_value);

// 查看频率
UART_Printf("Frequency: %.2f Hz\r\n", fg_frequency);
```

---

### 5. 常见问题

#### Q1: 怎么知道有哪些函数可用？

**A**：查看头文件
```c
// 打开 Drivers/STM32F1xx_HAL_Driver/Inc/stm32f1xx_hal_tim.h
// 搜索 "HAL_TIM_IC" 或 "Input Capture"
```

#### Q2: 函数参数怎么填？

**A**：查看函数原型和结构体定义
```c
// 函数原型
HAL_StatusTypeDef HAL_TIM_IC_ConfigChannel(TIM_HandleTypeDef *htim,
                                            TIM_IC_InitTypeDef *sConfigIC,
                                            uint32_t Channel);

// 结构体定义
typedef struct
{
  uint32_t ICPolarity;   // 看头文件中的宏定义
  uint32_t ICSelection;
  uint32_t ICPrescaler;
  uint32_t ICFilter;
} TIM_IC_InitTypeDef;
```

#### Q3: 回调函数在哪里实现？

**A**：在main.c中实现
```c
// 回调函数不需要在头文件中声明，直接实现即可
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
  // 实现代码
}
```

#### Q4: 如何知道函数是否成功？

**A**：检查返回值
```c
if (HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_2) != HAL_OK)
{
  UART_SendString("[ERROR] Failed to start\r\n");
  Error_Handler();
}
else
{
  UART_SendString("[INFO] Started successfully\r\n");
}
```

#### Q5: 怎么调试函数调用？

**A**：使用串口打印
```c
UART_SendString("[DEBUG] Before HAL_TIM_IC_Init\r\n");
HAL_TIM_IC_Init(&htim2);
UART_SendString("[DEBUG] After HAL_TIM_IC_Init\r\n");
```

---

### 6. 学习路径

```
第1周：理解基本概念
├─ 什么是输入捕获
├─ 什么是中断
└─ 什么是回调函数

第2周：学习HAL库
├─ 查看头文件
├─ 理解函数原型
└─ 学习结构体定义

第3周：实践练习
├─ 复制现有代码
├─ 修改参数测试
└─ 调试验证

第4周：独立开发
├─ 根据需求查找函数
├─ 编写新功能代码
└─ 调试优化
```

---

### 7. 总结

**新手查找函数的步骤**：

1. **明确需求**：要实现什么功能？
2. **选择外设**：用哪个外设（TIM/UART/I2C）？
3. **查看头文件**：找到对应的函数声明
4. **理解参数**：查看函数原型和结构体定义
5. **参考示例**：查看项目中的现有代码
6. **调试验证**：使用串口打印调试

**记住**：
- 所有函数都在头文件中声明
- 所有参数都在结构体中定义
- 所有示例都在项目代码中
- 所有问题都可以通过调试解决

**祝学习愉快！** 🎉

---

## 风扇控制模块流程图与代码对应详解

本章节详细说明边缘MCU软件设计文档v2.0.md中3.1风扇控制模块流程图的每个步骤与工程代码的对应关系。

---

### 流程图总览

```
┌─────────────────────────────────────────────────────────────────┐
│                        风扇控制模块流程图                         │
├─────────────────────────────────────────────────────────────────┤
│  初始化 → 主循环（命令解析）→ TIM2中断ISR → FG单次捕获处理       │
└─────────────────────────────────────────────────────────────────┘
```

---

### 一、初始化部分

#### 流程图步骤与代码对应表

| 流程图步骤 | 代码位置 | 文件 | 函数来源 |
|-----------|---------|------|---------|
| I1: 系统上电或复位 | `main()`函数入口 | main.c:795 | 自定义 |
| I2: 初始化GPIO/PWM/TIM捕获 | `MX_GPIO_Init()`等 | main.c:817-826 | 自定义+HAL库 |
| I3: 设置PWM=0%低电平 | `PWM_SetDutyPercent_ById()` | main.c:851-854 | 自定义 |
| I4: 启动TIM3 PWM输出 | `HAL_TIM_PWM_Start()` | main.c:846-849 | HAL库 |
| I5: 初始化FG捕获变量 | 全局变量定义 | main.c:220-230 | 自定义 |
| I6: 初始化SPI从机通信 | `MX_SPI2_Init()` | main.c:826 | 自定义+HAL库 |

#### 代码逐行解析

##### I1: 系统上电或复位

**位置**：`main.c` 第795行

```c
int main(void)
{
  /* USER CODE BEGIN 1 */
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/
  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();  // HAL库初始化
```

**逐行解释**：
| 行号 | 代码 | 含义 | 函数来源 |
|-----|------|------|---------|
| 795 | `int main(void)` | 程序入口函数，返回int类型，无参数 | C语言标准 |
| 805 | `HAL_Init();` | 初始化HAL库，配置Flash、SysTick定时器 | HAL库函数 |

##### I2: 初始化GPIO/PWM/TIM捕获

**位置**：`main.c` 第817-826行

```c
  /* Initialize all configured peripherals */
  MX_GPIO_Init();        // 初始化GPIO
  MX_USART1_UART_Init(); // 初始化USART1
  MX_USART2_UART_Init(); // 初始化USART2
  MX_TIM4_Init();        // 初始化TIM4（LED PWM）
  MX_TIM3_Init();        // 初始化TIM3（风扇PWM）
  MX_TIM2_Init();        // 初始化TIM2（FG输入捕获）
  MX_I2C_Init();         // 初始化I2C
  MX_SPI2_Init();        // 初始化SPI2从机
```

**逐行解释**：
| 行号 | 代码 | 含义 | 函数来源 |
|-----|------|------|---------|
| 817 | `MX_GPIO_Init();` | 初始化所有GPIO引脚 | 自定义（gpio.c） |
| 818 | `MX_USART1_UART_Init();` | 初始化USART1（115200bps，PC通信） | 自定义（usart.c） |
| 819 | `MX_USART2_UART_Init();` | 初始化USART2（9600bps，TEC通信） | 自定义（usart.c） |
| 820 | `MX_TIM4_Init();` | 初始化TIM4（LED PWM控制） | 自定义（tim.c） |
| 821 | `MX_TIM3_Init();` | 初始化TIM3（风扇/水泵PWM） | 自定义（tim.c） |
| 822 | `MX_TIM2_Init();` | 初始化TIM2（FG信号输入捕获） | 自定义（tim.c） |
| 823 | `MX_I2C_Init();` | 初始化I2C（TLA2528 ADC） | 自定义（i2c.c） |
| 826 | `MX_SPI2_Init();` | 初始化SPI2从机模式 | 自定义（spi.c） |

##### I3: 设置PWM=0%低电平

**位置**：`main.c` 第851-854行

```c
  /* 上电/复位后：所有风扇/水泵 PWM 置为 0%（低电平） */
  PWM_SetDutyPercent_ById(0, 0U);  // PUMP
  PWM_SetDutyPercent_ById(1, 0U);  // FAN1
  PWM_SetDutyPercent_ById(2, 0U);  // FAN2
  PWM_SetDutyPercent_ById(3, 0U);  // FAN3
```

**PWM_SetDutyPercent_ById函数详解**：

**位置**：`main.c` 第93-107行

```c
static void PWM_SetDutyPercent_ById(uint8_t pwm_id, uint8_t duty_percent)
{
    // 1. 根据pwm_id选择对应的TIM3通道
    uint32_t tim_channel = 0;
    if (pwm_id == 0) tim_channel = TIM_CHANNEL_1;      // PUMP → CH1
    else if (pwm_id == 1) tim_channel = TIM_CHANNEL_2; // FAN1 → CH2
    else if (pwm_id == 2) tim_channel = TIM_CHANNEL_3; // FAN2 → CH3
    else if (pwm_id == 3) tim_channel = TIM_CHANNEL_4; // FAN3 → CH4
    else return;  // 无效ID，直接返回

    // 2. 计算CCR值（比较寄存器值）
    uint32_t arr = __HAL_TIM_GET_AUTORELOAD(&htim3);  // 获取ARR值（999）
    uint32_t compare = ((arr + 1U) * duty_percent) / 100U;  // 计算CCR
    if (compare > arr) {
        compare = arr;  // 限制最大值
    }

    // 3. 设置CCR寄存器
    __HAL_TIM_SET_COMPARE(&htim3, tim_channel, compare);
}
```

**逐行解释**：
| 行号 | 代码 | 含义 | 函数来源 |
|-----|------|------|---------|
| 93 | `static void PWM_SetDutyPercent_ById(...)` | 静态函数，仅本文件可见 | 自定义 |
| 96-99 | `if (pwm_id == 0)...` | 将逻辑ID映射到TIM通道 | 自定义逻辑 |
| 102 | `__HAL_TIM_GET_AUTORELOAD(&htim3)` | 获取TIM3的ARR寄存器值 | HAL库宏 |
| 103 | `((arr + 1U) * duty_percent) / 100U` | 计算占空比对应的CCR值 | 自定义计算 |
| 107 | `__HAL_TIM_SET_COMPARE(...)` | 设置TIM3的CCR寄存器 | HAL库宏 |

##### I4: 启动TIM3 PWM输出

**位置**：`main.c` 第846-849行

```c
  /* 启动 TIM3 PWM（PC6~PC9：PUMP/FAN1/FAN2/FAN3，TIM3启用FULL REMAP） */
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_4);
```

**逐行解释**：
| 行号 | 代码 | 含义 | 函数来源 |
|-----|------|------|---------|
| 846 | `HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1)` | 启动TIM3通道1 PWM输出 | HAL库函数 |
| 847-849 | 同上 | 启动其他通道 | HAL库函数 |

**HAL_TIM_PWM_Start函数原型**：
```c
// 位置：Drivers/STM32F1xx_HAL_Driver/Inc/stm32f1xx_hal_tim.h
HAL_StatusTypeDef HAL_TIM_PWM_Start(TIM_HandleTypeDef *htim, uint32_t Channel);
```

##### I5: 初始化FG捕获变量

**位置**：`main.c` 第220-230行

```c
static volatile uint32_t fg_capture_count = 0;    // FG边沿捕获计数
static volatile float fg_frequency = 0.0f;        // 计算得到的FG频率(Hz)
static volatile uint32_t fg_last_time = 0;        // 上次捕获时间
static volatile uint32_t fg_current_time = 0;     // 当前捕获时间
static volatile uint32_t fg_period = 0;           // 两次捕获周期
static uint8_t fg_one_shot_active = 0;            // 单次FG捕获激活标志
static uint32_t fg_one_shot_start_tick = 0;       // 单次捕获开始时刻
static uint32_t fg_one_shot_settle_until = 0;     // MUX稳定等待截止时间
static uint32_t fg_one_shot_timeout_ms = 2000U;   // 单次捕获超时时间
```

**逐行解释**：
| 行号 | 变量声明 | 含义 | 关键字说明 |
|-----|---------|------|-----------|
| 220 | `static volatile uint32_t` | 静态、易变、无符号32位整数 | `static`仅本文件可见，`volatile`防止编译器优化 |
| 221 | `float` | 单精度浮点数 | 用于存储频率计算结果 |

##### I6: 初始化SPI从机通信

**位置**：`main.c` 第826行，`spi.c` 第92-119行

```c
// main.c
MX_SPI2_Init();  // SPI2 从机：PB12~PB15

// spi.c - MX_SPI2_Init()
void MX_SPI2_Init(void)
{
  hspi2.Instance = SPI2;                      // 选择SPI2外设
  hspi2.Init.Mode = SPI_MODE_SLAVE;           // 从机模式
  hspi2.Init.Direction = SPI_DIRECTION_2LINES; // 全双工
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;    // 8位数据
  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;  // CPOL=0
  hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;      // CPHA=0
  hspi2.Init.NSS = SPI_NSS_HARD_INPUT;        // 硬件NSS输入
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;     // MSB优先
  // ...
  if (HAL_SPI_Init(&hspi2) != HAL_OK) {
    Error_Handler();
  }
}
```

**SPI引脚配置**：
| 引脚 | 功能 | 方向 |
|-----|------|------|
| PB12 | NSS（片选） | 输入 |
| PB13 | SCK（时钟） | 输入 |
| PB14 | MISO（主入从出） | 输出 |
| PB15 | MOSI（主出从入） | 输入 |

---

### 二、主循环部分

#### 流程图步骤与代码对应表

| 流程图步骤 | 代码位置 | 文件 | 函数来源 |
|-----------|---------|------|---------|
| M1: 等待接收SPI命令 | `while(1)`循环内 | main.c:990 | 自定义 |
| M2: 解析命令 | `if-else if`链 | main.c:1008-1130 | 自定义 |
| M3-M5: xPWMxxx处理 | `parse_xpwm_cmd()` | main.c:1084-1097 | 自定义 |
| M7-M11: FGRDxxxx处理 | `parse_fgrd_cmd()` | main.c:1056-1081 | 自定义 |
| M12-M14: FG_RD转速读取 | 待实现 | - | - |

#### 代码逐行解析

##### M1: 等待接收SPI命令

**位置**：`main.c` 第990-1008行

```c
  while (1)
  {
    /* Check USART1 RX - Process received command */
    uint8_t rx_data[64];
    uint8_t len = USART1_GetReceivedData(rx_data, sizeof(rx_data));
    if (len > 0)
    {
      // 命令处理...
    }
```

**逐行解释**：
| 行号 | 代码 | 含义 | 函数来源 |
|-----|------|------|---------|
| 990 | `while (1)` | 无限循环，程序主循环 | C语言标准 |
| 993 | `uint8_t rx_data[64]` | 定义64字节接收缓冲区 | 自定义 |
| 994 | `USART1_GetReceivedData(...)` | 获取USART1接收到的数据 | 自定义（usart.c） |
| 995 | `if (len > 0)` | 判断是否收到数据 | C语言标准 |

##### M3-M5: xPWMxxx命令处理

**位置**：`main.c` 第1084-1097行

```c
      /* xPWMoff% 指令：在线配置 PUMP/FAN PWM（off%=xxx, duty=100-off） */
      else if (parse_xpwm_cmd((const char *)rx_data, &pwm_id_cfg, &off_percent_cfg))
      {
        // 1. 计算实际占空比
        uint8_t duty_on_percent = (uint8_t)(100U - off_percent_cfg);

        // 2. 设置PWM占空比
        PWM_SetDutyPercent_ById(pwm_id_cfg, duty_on_percent);

        // 3. 确定TIM通道用于打印
        uint32_t tim_channel = TIM_CHANNEL_1;
        if (pwm_id_cfg == 1) tim_channel = TIM_CHANNEL_2;
        else if (pwm_id_cfg == 2) tim_channel = TIM_CHANNEL_3;
        else if (pwm_id_cfg == 3) tim_channel = TIM_CHANNEL_4;

        // 4. 打印结果
        UART_Printf("[OK] xPWM%u off=%u%% duty=%u%% (CCR=%lu/ARR=%lu)\r\n",
                    pwm_id_cfg, off_percent_cfg, duty_on_percent,
                    (uint32_t)__HAL_TIM_GET_COMPARE(&htim3, tim_channel),
                    (uint32_t)__HAL_TIM_GET_AUTORELOAD(&htim3) + 1U);
      }
```

**parse_xpwm_cmd函数详解**：

**位置**：`main.c` 第124-153行

```c
static uint8_t parse_xpwm_cmd(const char *cmd, uint8_t *out_pwm_id, uint8_t *out_off_percent)
{
    // 1. 参数检查
    if (cmd == NULL || out_pwm_id == NULL || out_off_percent == NULL) return 0;
    
    // 2. 长度检查：必须是7个字符（"xPWMxxx"）
    if (strlen(cmd) != 7) return 0;

    // 3. 检查第一个字符是否为数字0-3
    if (!isdigit((unsigned char)cmd[0])) return 0;
    uint8_t pwm_id = (uint8_t)(cmd[0] - '0');  // 字符转数字
    if (pwm_id > 3) return 0;  // 只允许0-3

    // 4. 检查"PWM"字符串
    if (toupper((unsigned char)cmd[1]) != 'P' ||
        toupper((unsigned char)cmd[2]) != 'W' ||
        toupper((unsigned char)cmd[3]) != 'M') {
        return 0;
    }

    // 5. 检查后三位是否为数字
    if (!isdigit((unsigned char)cmd[4]) ||
        !isdigit((unsigned char)cmd[5]) ||
        !isdigit((unsigned char)cmd[6])) {
        return 0;
    }

    // 6. 解析关断百分比
    uint16_t off = (uint16_t)(cmd[4] - '0') * 100U +   // 百位
                    (uint16_t)(cmd[5] - '0') * 10U +    // 十位
                    (uint16_t)(cmd[6] - '0');           // 个位
    if (off > 100U) return 0;  // 最大100%

    // 7. 输出结果
    *out_pwm_id = pwm_id;
    *out_off_percent = (uint8_t)off;
    return 1;  // 解析成功
}
```

**逐行解释**：
| 行号 | 代码 | 含义 | 函数来源 |
|-----|------|------|---------|
| 126 | `if (cmd == NULL...)` | 空指针检查，防止崩溃 | C语言标准 |
| 129 | `strlen(cmd) != 7` | 字符串长度检查 | C标准库 |
| 132 | `isdigit()` | 检查是否为数字字符 | C标准库 |
| 133 | `cmd[0] - '0'` | 字符转数字（ASCII码差值） | C语言技巧 |
| 136-138 | `toupper()` | 转大写比较 | C标准库 |

##### M7-M11: FGRDxxxx命令处理

**位置**：`main.c` 第1056-1081行

```c
      /* FGRDxxxx：切换 MUX 通道并回读一次 */
      else if (parse_fgrd_cmd((const char *)rx_data, &en0, &en1, &a0, &a1, &a2))
      {
        /* 先切换 MUX，再重置FG捕获统计 */
        MUX_SetFGMuxPins(en0, en1, a0, a1, a2);  // M7: 切换MUX

        // M9: 重置FG捕获统计
        fg_one_shot_active = 1;
        fg_one_shot_start_tick = HAL_GetTick();
        fg_one_shot_settle_until = fg_one_shot_start_tick + 20U; // M10: 20ms稳定时间

        // 保存参数用于打印
        fg_one_shot_en0 = en0;
        fg_one_shot_en1 = en1;
        fg_one_shot_addr0 = a0;
        fg_one_shot_addr1 = a1;
        fg_one_shot_addr2 = a2;

        // 重置捕获变量
        fg_capture_count = 0;
        fg_frequency = 0.0f;
        fg_last_time = 0;
        fg_current_time = 0;
        fg_period = 0;
        last_fg_update_time = fg_one_shot_start_tick;

        UART_SendString("[OK] FGRD pending capture\r\n");  // M11: 启动单次捕获
      }
```

**MUX_SetFGMuxPins函数详解**：

**位置**：`main.c` 第240-255行

```c
void MUX_SetFGMuxPins(uint8_t en0, uint8_t en1, 
                      uint8_t a0, uint8_t a1, uint8_t a2)
{
    // 设置EN0和EN1（互斥）
    HAL_GPIO_WritePin(MUX_EN0_GPIO_Port, MUX_EN0_Pin, 
                      en0 ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MUX_EN1_GPIO_Port, MUX_EN1_Pin, 
                      en1 ? GPIO_PIN_SET : GPIO_PIN_RESET);
    
    // 设置地址线A0, A1, A2
    HAL_GPIO_WritePin(MUX_ADDR0_GPIO_Port, MUX_ADDR0_Pin, 
                      a0 ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MUX_ADDR1_GPIO_Port, MUX_ADDR1_Pin, 
                      a1 ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MUX_ADDR2_GPIO_Port, MUX_ADDR2_Pin, 
                      a2 ? GPIO_PIN_SET : GPIO_PIN_RESET);
}
```

**逐行解释**：
| 行号 | 代码 | 含义 | 函数来源 |
|-----|------|------|---------|
| 242 | `HAL_GPIO_WritePin(...)` | 设置GPIO引脚电平 | HAL库函数 |
| 243 | `en0 ? GPIO_PIN_SET : GPIO_PIN_RESET` | 三目运算符，en0非0输出高电平 | C语言标准 |

---

### 三、TIM2输入捕获中断ISR

#### 流程图步骤与代码对应表

| 流程图步骤 | 代码位置 | 文件 | 函数来源 |
|-----------|---------|------|---------|
| T1: FG边沿触发中断 | 硬件自动触发 | - | 硬件 |
| T2: 读取CCR2捕获值 | `HAL_TIM_ReadCapturedValue()` | main.c:277 | HAL库 |
| T3: 判断是否首次捕获 | `if (fg_capture_count == 0)` | main.c:287 | 自定义 |
| T4-T5: 记录首次捕获 | 变量赋值 | main.c:289-291 | 自定义 |
| T6-T7: 更新时间戳 | 变量赋值 | main.c:301-302 | 自定义 |
| T8-T10: 计算周期 | 条件判断 | main.c:304-312 | 自定义 |
| T11: 计算频率 | 浮点除法 | main.c:315-316 | 自定义 |
| T12-T13: 更新计数和时间 | 变量赋值 | main.c:318-333 | 自定义 |

#### 代码逐行解析

**位置**：`main.c` 第270-340行

```c
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    // 1. 检查是否为TIM2通道2
    if (htim->Instance == TIM2 && htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)
    {
        static uint32_t callback_count = 0;  // 静态计数器
        callback_count++;
			
        // T2: 读取CCR2捕获值
        uint32_t captured_value = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2);

        // Debug打印（前几次）
        if (fg_debug_enabled && callback_count <= 5)
        {
            UART_Printf("[FG DEBUG] Callback #%lu: TIM2_CH2, Value=%lu\r\n",
                        callback_count, captured_value);
        }

        // T3: 判断是否首次捕获
        if (fg_capture_count == 0)
        {
            // T4: 记录首次捕获值
            fg_last_time = captured_value;
            fg_current_time = captured_value;
            fg_capture_count++;
            last_fg_update_time = HAL_GetTick();
            return;  // 首次捕获直接返回
        }

        // T6-T7: 更新时间戳
        fg_last_time = fg_current_time;
        fg_current_time = captured_value;

        // T8-T10: 计算周期（处理溢出）
        if (fg_current_time > fg_last_time)
        {
            fg_period = fg_current_time - fg_last_time;
        }
        else
        {
            // 计数器溢出处理：0xFFFF - last + current + 1
            fg_period = (0xFFFF - fg_last_time) + fg_current_time + 1;
        }

        // T11: 计算频率
        if (fg_period > 0)
        {
            // TIM2时钟72MHz，PSC=71，实际计数频率=1MHz
            // 但代码中使用10kHz，可能是PSC=7199
            fg_frequency = 10000.0f / fg_period;
        }
        else
        {
            fg_frequency = 0.0f;
        }

        // T12: 增加捕获计数
        fg_capture_count++;

        // T13: 更新最后捕获时间
        last_fg_update_time = HAL_GetTick();
    }
}
```

**逐行解释**：
| 行号 | 代码 | 含义 | 函数来源 |
|-----|------|------|---------|
| 270 | `void HAL_TIM_IC_CaptureCallback(...)` | HAL库输入捕获回调函数 | HAL库约定 |
| 272 | `htim->Instance == TIM2` | 判断是否为TIM2 | HAL库结构体 |
| 274 | `static uint32_t` | 静态变量，函数结束后值保留 | C语言标准 |
| 277 | `HAL_TIM_ReadCapturedValue()` | 读取捕获寄存器值 | HAL库函数 |
| 304-312 | `if (fg_current_time > fg_last_time)` | 处理计数器溢出 | 自定义逻辑 |
| 315 | `10000.0f / fg_period` | 计算频率（Hz） | 自定义计算 |

---

### 四、FG单次捕获处理（主循环轮询）

#### 流程图步骤与代码对应表

| 流程图步骤 | 代码位置 | 文件 | 函数来源 |
|-----------|---------|------|---------|
| F1: fg_one_shot_active? | `if (fg_one_shot_active)` | main.c:1158 | 自定义 |
| F2: 当前时间>=settle_until? | `if (now >= fg_one_shot_settle_until)` | main.c:1161 | 自定义 |
| F3: fg_capture_count>=2? | `if (fg_capture_count >= 2...)` | main.c:1166 | 自定义 |
| F4: 计算RPM | `fan_rpm = freq_to_report * 30.0f` | main.c:1176 | 自定义 |
| F5: 打印结果 | `UART_Printf()` | main.c:1179 | 自定义 |
| F6: fg_one_shot_active=0 | `fg_one_shot_active = 0` | main.c:1185 | 自定义 |
| F7: 超时检查 | `time_since_last_capture > timeout` | main.c:1167 | 自定义 |

#### 代码逐行解析

**位置**：`main.c` 第1158-1187行

```c
    /* FGRDxxxx 一键回读一次：等待捕获到足够边沿后打印 */
    if (fg_one_shot_active)  // F1: 检查单次捕获是否激活
    {
      uint32_t now = HAL_GetTick();
      
      // F2: 检查是否已过MUX稳定时间
      if (now >= fg_one_shot_settle_until)
      {
        uint32_t time_since_last_capture = now - last_fg_update_time;

        // F3+F7: 检查捕获条件（>=2个边沿 或 超时）
        if (fg_capture_count >= 2 ||
            time_since_last_capture > fg_one_shot_timeout_ms ||
            (now - fg_one_shot_start_tick) > fg_one_shot_timeout_ms)
        {
          // 确定要报告的频率
          float freq_to_report = fg_frequency;
          if (fg_capture_count < 2 || time_since_last_capture > fg_one_shot_timeout_ms)
          {
            freq_to_report = 0.0f;  // 超时或捕获不足，报告0
          }

          // F4: 计算RPM = 30 * 频率
          float fan_rpm = freq_to_report * 30.0f;
          uint32_t captures_to_report = fg_capture_count;

          // F5: 打印结果
          UART_Printf("[FGRD] PA1(TIM2_CH2) MUX(EN0=%u,EN1=%u,A0=%u,A1=%u,A2=%u) "
                      "Freq: %.2f Hz, RPM: %.0f, Captures: %lu\r\n",
                      fg_one_shot_en0, fg_one_shot_en1,
                      fg_one_shot_addr0, fg_one_shot_addr1, fg_one_shot_addr2,
                      freq_to_report, fan_rpm, captures_to_report);

          // F6: 结束单次捕获
          fg_one_shot_active = 0;
        }
      }
    }
```

**逐行解释**：
| 行号 | 代码 | 含义 | 函数来源 |
|-----|------|------|---------|
| 1158 | `if (fg_one_shot_active)` | 检查单次捕获标志 | 自定义 |
| 1160 | `HAL_GetTick()` | 获取系统滴答计数（ms） | HAL库函数 |
| 1161 | `now >= fg_one_shot_settle_until` | 判断MUX稳定时间是否已到 | 自定义逻辑 |
| 1166-1168 | 多条件或判断 | 捕获成功或超时 | 自定义逻辑 |
| 1176 | `freq_to_report * 30.0f` | RPM = 60*f/2 = 30*f | 自定义公式 |

---

### 五、主从MCU的SPI通信详解

#### 1. SPI通信架构

```
┌──────────────┐                    ┌──────────────┐
│   主MCU      │                    │  边缘MCU     │
│  (Master)    │                    │  (Slave)     │
├──────────────┤                    ├──────────────┤
│ SPI2_NSS  ───┼───── PB12 ─────────┼─► SPI2_NSS   │
│ SPI2_SCK  ───┼───── PB13 ─────────┼─► SPI2_SCK   │
│ SPI2_MISO ◄──┼───── PB14 ─────────┼─── SPI2_MISO │
│ SPI2_MOSI ───┼───── PB15 ─────────┼─► SPI2_MOSI  │
└──────────────┘                    └──────────────┘
```

#### 2. SPI从机初始化

**位置**：`spi.c` 第92-119行

```c
void MX_SPI2_Init(void)
{
  hspi2.Instance = SPI2;                      // SPI2外设
  hspi2.Init.Mode = SPI_MODE_SLAVE;           // 从机模式
  hspi2.Init.Direction = SPI_DIRECTION_2LINES; // 全双工
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;    // 8位数据帧
  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;  // CPOL=0（空闲低电平）
  hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;      // CPHA=0（第一边沿采样）
  hspi2.Init.NSS = SPI_NSS_HARD_INPUT;        // 硬件NSS输入
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;     // MSB优先传输
  
  if (HAL_SPI_Init(&hspi2) != HAL_OK) {
    Error_Handler();
  }
}
```

**SPI模式说明**：
| 参数 | 值 | 含义 |
|-----|---|------|
| CPOL | 0 | 空闲时时钟为低电平 |
| CPHA | 0 | 第一边沿（上升沿）采样数据 |
| Mode | Slave | 从机模式，被动响应主机 |

#### 3. SPI中断处理机制

**位置**：`spi.c` 第299-398行

```c
void SPI2_SlaveIRQHandler(void)
{
  SPI_TypeDef *spi = hspi2.Instance;
  uint16_t sr = spi->SR;  // 读取状态寄存器

  // 1. RXNE中断：接收到数据
  if ((sr & SPI_SR_RXNE) != 0U) {
    uint8_t in = (uint8_t)spi->DR;  // 读取数据寄存器（自动清除RXNE）

    if (spi2_state == SPI2_ST_RX) {
      // 接收状态机
      if (in == '\r') {
        // 收到回车，准备响应
        spi2_seen_cr = 1;
        spi2_prepare_response_and_uart();
        // 预装首字节到DR，等待'\n'
        if ((spi->SR & SPI_SR_TXE) != 0U) {
          spi->DR = spi2_tx_buf[spi2_tx_idx++];
        }
      } else if (in == '\n') {
        // 收到换行，开始发送响应
        spi2_state = SPI2_ST_TX;
        spi2_enable_txe_irq();
        // 立即预装首字节
        if ((spi->SR & SPI_SR_TXE) != 0U) {
          spi->DR = spi2_tx_buf[spi2_tx_idx++];
        }
      } else {
        // 普通字符，存入接收缓冲区
        if (spi2_rx_len < SPI2_MSG_MAX) {
          spi2_rx_buf[spi2_rx_len++] = in;
        }
      }
    }
  }

  // 2. TXE中断：发送缓冲区空
  if ((sr & SPI_SR_TXE) != 0U) {
    if (spi2_state == SPI2_ST_TX) {
      if (spi2_tx_idx < spi2_tx_len) {
        spi->DR = spi2_tx_buf[spi2_tx_idx++];  // 发送下一字节
      } else {
        spi->DR = 0x00;  // 发送完成，发0x00占位
        spi2_disable_txe_irq();
        spi2_state = SPI2_ST_POST;
      }
    }
  }
}
```

#### 4. SPI状态机

```
┌─────────┐    收到首字节    ┌─────────┐    收到'\n'    ┌─────────┐
│  IDLE   │ ──────────────► │   RX    │ ─────────────► │   TX    │
└─────────┘                 └─────────┘                └─────────┘
     ▲                            │                         │
     │                            │ 收到'\r'                │ 发送完成
     │                            ▼                         │
     │                     ┌─────────────┐                  │
     │                     │ TX_WAIT_LF  │                  │
     │                     └─────────────┘                  │
     │                            │                         │
     │                            │ NSS上升沿               │
     └────────────────────────────┴─────────────────────────┘
```

#### 5. SPI通信协议

**命令格式**：以`\r\n`结尾的ASCII字符串

```
主机发送: "0PWM050\r\n"
从机响应: "0PWM050+\r\n"（原样返回加'+'表示确认）
```

**关键函数调用关系**：

| 函数 | 文件 | 来源 | 作用 |
|-----|------|------|------|
| `MX_SPI2_Init()` | spi.c | 自定义 | 初始化SPI2外设 |
| `HAL_SPI_Init()` | HAL库 | ST官方 | HAL库SPI初始化 |
| `SPI2_SlaveComm_Start()` | spi.c | 自定义 | 启动SPI从机通信 |
| `SPI2_SlaveIRQHandler()` | spi.c | 自定义 | SPI中断处理 |
| `SPI2_SlaveComm_ReadUart()` | spi.c | 自定义 | 读取SPI接收的数据 |

---

### 六、函数来源汇总表

#### 自定义函数（项目自己编写）

| 函数名 | 文件 | 功能 |
|-------|------|------|
| `PWM_SetDutyPercent_ById()` | main.c | 设置单路PWM占空比 |
| `parse_xpwm_cmd()` | main.c | 解析xPWMxxx命令 |
| `parse_fgrd_cmd()` | main.c | 解析FGRDxxxx命令 |
| `MUX_SetFGMuxPins()` | main.c | 设置MUX选择引脚 |
| `UART_SendString()` | main.c | 发送字符串到串口 |
| `UART_Printf()` | main.c | 格式化打印到串口 |
| `SPI2_SlaveComm_Start()` | spi.c | 启动SPI从机通信 |
| `SPI2_SlaveIRQHandler()` | spi.c | SPI中断处理 |

#### HAL库函数（ST官方提供）

| 函数名 | 头文件 | 功能 |
|-------|--------|------|
| `HAL_Init()` | stm32f1xx_hal.h | HAL库初始化 |
| `HAL_TIM_PWM_Start()` | stm32f1xx_hal_tim.h | 启动PWM输出 |
| `HAL_TIM_IC_Start_IT()` | stm32f1xx_hal_tim.h | 启动输入捕获中断 |
| `HAL_TIM_ReadCapturedValue()` | stm32f1xx_hal_tim.h | 读取捕获值 |
| `HAL_GPIO_WritePin()` | stm32f1xx_hal_gpio.h | 设置GPIO电平 |
| `HAL_UART_Transmit()` | stm32f1xx_hal_uart.h | 串口发送数据 |
| `HAL_UART_Receive_IT()` | stm32f1xx_hal_uart.h | 启动串口中断接收 |
| `HAL_SPI_Init()` | stm32f1xx_hal_spi.h | SPI初始化 |
| `HAL_GetTick()` | stm32f1xx_hal.h | 获取系统滴答计数 |

#### C标准库函数

| 函数名 | 头文件 | 功能 |
|-------|--------|------|
| `strlen()` | string.h | 计算字符串长度 |
| `isdigit()` | ctype.h | 检查是否为数字 |
| `toupper()` | ctype.h | 转大写 |
| `memcpy()` | string.h | 内存拷贝 |
| `memset()` | string.h | 内存填充 |

---

### 七、关键宏定义

#### HAL库宏（寄存器操作封装）

| 宏名 | 功能 | 示例 |
|-----|------|------|
| `__HAL_TIM_SET_COMPARE()` | 设置CCR寄存器 | `__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 500)` |
| `__HAL_TIM_GET_AUTORELOAD()` | 获取ARR寄存器 | `uint32_t arr = __HAL_TIM_GET_AUTORELOAD(&htim3)` |
| `__HAL_TIM_GET_COMPARE()` | 获取CCR寄存器 | `uint32_t ccr = __HAL_TIM_GET_COMPARE(&htim3, TIM_CHANNEL_1)` |
| `__HAL_RCC_GPIOA_CLK_ENABLE()` | 使能GPIOA时钟 | 初始化时调用 |

#### 项目自定义宏

| 宏名 | 值 | 含义 |
|-----|---|------|
| `SPI2_MSG_MAX` | 128 | SPI消息最大长度 |
| `SPI2_TXQ_SIZE` | 256 | SPI发送队列大小 |
| `NTC_CHANNEL` | 0 | NTC温度传感器ADC通道 |
| `NTC_ADC_MAX` | 4095 | ADC最大值（12位） |
| `NTC_VCC` | 3.3f | NTC供电电压 |
| `NTC_RREF` | 10000.0f | NTC参考电阻（10kΩ） |

---

## 主从MCU SPI通信协议

### 通信架构

```
┌──────────────┐                    ┌──────────────┐
│   主MCU      │                    │  边缘MCU     │
│  (Master)    │                    │  (Slave)     │
├──────────────┤                    ├──────────────┤
│ SPI2_NSS  ───┼───── PB12 ─────────┼─► SPI2_NSS   │
│ SPI2_SCK  ───┼───── PB13 ─────────┼─► SPI2_SCK   │
│ SPI2_MISO ◄──┼───── PB14 ─────────┼─── SPI2_MISO │
│ SPI2_MOSI ───┼───── PB15 ─────────┼─► SPI2_MOSI  │
└──────────────┘                    └──────────────┘
```

### SPI配置参数

| 参数 | 值 | 说明 |
|-----|---|------|
| 模式 | Slave | 从机模式 |
| 时钟极性(CPOL) | 0 | 空闲时SCK为低电平 |
| 时钟相位(CPHA) | 0 | 第一边沿采样 |
| 数据位宽 | 8bit | 每帧8位 |
| 位序 | MSB | 高位优先 |
| NSS | 硬件输入 | 硬件片选 |

### 通信流程

```
主机发送命令 ──► 从机接收并存储命令
     │                │
     │                ▼
     │         从机返回"PENDING"
     │                │
     ▼                ▼
主机再次发起通信 ◄─── 从机处理命令并设置响应
     │
     ▼
从机返回执行结果
```

### 支持的SPI命令

#### 1. PWM控制命令：xPWMxxx

**格式**：`xPWMxxx\r\n`

| 字段 | 含义 | 取值范围 |
|-----|------|---------|
| x | PWM通道ID | 0=PUMP, 1=FAN1, 2=FAN2, 3=FAN3 |
| xxx | 关断百分比(off%) | 000~100 |

**示例**：
```
主机发送: "0PWM050\r\n"    // PUMP PWM占空比设为50%
从机响应: "OK PWM0=50%\r\n"
```

#### 2. FG通道选择命令：FGRDxxxx

**格式**：`FGRDxxxx\r\n`

| 字段 | 含义 | 取值范围 |
|-----|------|---------|
| x(第1位) | EN选择 | 0=EN0有效, 1=EN1有效 |
| x(第2位) | ADDR0 | 0或1 |
| x(第3位) | ADDR1 | 0或1 |
| x(第4位) | ADDR2 | 0或1 |

**示例**：
```
主机发送: "FGRD0100\r\n"   // EN0=0, EN1=1, ADDR=000
从机响应: "OK FGRD EN0=0 EN1=1 A=000\r\n"
```

#### 3. FG转速读取命令：FG_RD / GET_FG

**格式**：`FG_RD\r\n` 或 `GET_FG\r\n`

**示例**：
```
主机发送: "FG_RD\r\n"
从机响应: "FG Freq=25.00Hz RPM=750 Cap=10\r\n"
```

**响应字段说明**：
| 字段 | 含义 |
|-----|------|
| Freq | FG信号频率(Hz) |
| RPM | 风扇转速(转/分钟) |
| Cap | 捕获边沿计数 |

#### 4. PWM状态查询命令：GET_PWM

**格式**：`GET_PWM\r\n`

**示例**：
```
主机发送: "GET_PWM\r\n"
从机响应: "PWM ARR=1000 CCR=500,300,200,100\r\n"
```

### 主机端实现示例

#### 发送命令函数

```c
void SPI_SendCommand(const char *cmd, char *response, uint16_t resp_max_len)
{
    uint16_t cmd_len = strlen(cmd);
    
    // 拉低NSS
    HAL_GPIO_WritePin(SPI_NSS_PORT, SPI_NSS_PIN, GPIO_PIN_RESET);
    HAL_Delay(1);
    
    // 发送命令
    HAL_SPI_TransmitReceive(&hspi, (uint8_t*)cmd, (uint8_t*)response, cmd_len, 100);
    
    // 拉高NSS
    HAL_GPIO_WritePin(SPI_NSS_PORT, SPI_NSS_PIN, GPIO_PIN_SET);
    HAL_Delay(10);  // 等待从机处理
    
    // 再次通信获取响应
    HAL_GPIO_WritePin(SPI_NSS_PORT, SPI_NSS_PIN, GPIO_PIN_RESET);
    HAL_Delay(1);
    
    memset(response, 0, resp_max_len);
    HAL_SPI_TransmitReceive(&hspi, dummy_tx, (uint8_t*)response, resp_max_len, 100);
    
    HAL_GPIO_WritePin(SPI_NSS_PORT, SPI_NSS_PIN, GPIO_PIN_SET);
}
```

#### 使用示例

```c
char response[128];

// 设置PUMP PWM为50%
SPI_SendCommand("0PWM050\r\n", response, sizeof(response));
printf("Response: %s", response);  // 输出: OK PWM0=50%

// 选择FG通道
SPI_SendCommand("FGRD0100\r\n", response, sizeof(response));
printf("Response: %s", response);  // 输出: OK FGRD EN0=0 EN1=1 A=000

// 等待FG捕获稳定
HAL_Delay(100);

// 读取转速
SPI_SendCommand("FG_RD\r\n", response, sizeof(response));
printf("Response: %s", response);  // 输出: FG Freq=25.00Hz RPM=750 Cap=10
```

### 错误响应

当发送无法识别的命令时：

```
主机发送: "UNKNOWN\r\n"
从机响应: "ERR Unknown cmd: UNKNOWN\r\n"
```

### 时序要求

| 操作 | 最小延时 |
|-----|---------|
| NSS拉低到SCK开始 | 1ms |
| 命令发送完成到读取响应 | 10ms |
| FGRD命令后等待FG捕获 | 100ms |
| NSS拉高到下一次通信 | 1ms |

### 注意事项

1. **NSS控制**：主机必须在每次通信前拉低NSS，通信结束后拉高
2. **命令结束符**：所有命令必须以`\r\n`结尾
3. **响应时机**：从机首次返回`PENDING`，主机需要再次发起通信获取实际结果
4. **超时处理**：建议主机端设置500ms超时，防止从机无响应时阻塞

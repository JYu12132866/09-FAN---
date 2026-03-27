# SPI 主机通信协议原理、工程实现与问题修复记录

本文档用于说明本工程中 **STM32 作为 SPI 主机（Master）** 的通信协议原理、代码实现方式，以及“串口助手下发字符串需要完整传给 SPI 并正确打印回串口助手”问题的分析与修复过程。

---

## SPI 主机通信协议原理（面向工程实现）

### 1) SPI 的基本信号与角色

SPI（Serial Peripheral Interface）通常包含 4 根信号线：

- **SCK**：时钟（由主机输出）
- **MOSI**：主机输出/从机输入
- **MISO**：主机输入/从机输出
- **NSS/CS**：片选（通常主机输出，低电平有效）

角色划分：

- **主机 Master**：产生 SCK，控制 NSS，决定何时开始/结束一帧传输。
- **从机 Slave**：在 NSS 有效期间，跟随 SCK 在 MOSI/MISO 上移位收发数据。

### 2) SPI 的“帧”与全双工特性

SPI 是**同步串行**，一次传输的最小单位是“时钟边沿上的移位”：

- 主机每输出 1 个时钟周期，就会在 MOSI 上“送出 1bit”，同时在 MISO 上“采样 1bit”。
- 因此 SPI 天然是**全双工**：发送多少字节，就会同时接收多少字节（收到什么取决于从机在同一时刻输出了什么）。

工程上常见两种等价理解：

- **发送 N 字节**：同时会收到 N 字节（即便从机回的是无意义数据，也会有接收缓存被填充）。
- **接收 N 字节**：主机必须同时发送 N 字节“占位数据”（常用 0x00/0xFF），因为没有时钟就不会移位。

### 3) 时序参数：CPOL/CPHA（SPI Mode）

SPI 的时序由两个参数决定：

- **CPOL**：时钟空闲电平（LOW/HIGH）
- **CPHA**：采样边沿（第一个边沿/第二个边沿）

常用写法：Mode0/1/2/3。

本工程 `SPI2` 初始化为：

- **CPOL = LOW**
- **CPHA = 1EDGE**

即常说的 **SPI Mode 0**（空闲低电平、在第一个边沿采样）。

### 4) NSS/CS（片选）的工程含义

从机通常通过 NSS/CS 判断：

- “什么时候开始一帧”（CS 拉低）
- “什么时候结束一帧”（CS 拉高）

如果 CS 控制不正确（例如拉低太短、在帧中抖动、或与从机协议不匹配），会导致：

- 从机状态机错帧
- 接收/发送数据错位
- 读到“看似乱码”的数据

本工程使用 **软件控制片选（Soft NSS）**：GPIO 手动拉低/拉高 CS。

---

## 本工程 SPI 主机实现（SPI2）

### 1) SPI2 硬件配置概览

工程文件：`Src/spi.c`

- **主机模式**：`SPI_MODE_MASTER`
- **方向**：`SPI_DIRECTION_2LINES`（MOSI/MISO 双线）
- **数据位宽**：`SPI_DATASIZE_8BIT`
- **时序**：`CPOL=LOW, CPHA=1EDGE`（Mode 0）
- **片选**：`SPI_NSS_SOFT`（软件片选）
- **分频**：`SPI_BAUDRATEPRESCALER_64`
- **CS 引脚**：`PB12` 配置成 GPIO 输出（低有效）

### 2) 关键收发函数：全双工 `SPI2_TransmitReceive`

工程文件：`Src/spi.c`

函数：

- `HAL_StatusTypeDef SPI2_TransmitReceive(uint8_t *tx_data, uint8_t *rx_data, uint16_t size)`

流程要点：

- 发送前清空 `rx_data`
- CS 拉低
- 调用 `HAL_SPI_TransmitReceive()` 同时收发 `size` 字节
- CS 拉高

注意：

- SPI 全双工下，“接收缓冲区的内容”由从机决定；若从机未按协议回传/未回环，`SPI RX` 不等于 `SPI TX` 是正常现象。

---

## 串口→SPI→串口打印 需求与问题现象

### 需求

- 串口助手每次下发的一串字符，都要：
  1) **完整**进入 MCU；
  2) **完整**送入 SPI 发送；
  3) 通过串口打印 `SPI TX` 与 `SPI RX`（以及回显/提示信息）；
  4) 不出现“缺字、错位、乱码（由丢字引起）”等异常。

### 现象（典型）

- 同样输入一行字符串，有时 MCU 收到/回显/用于 SPI 的字符串会变短（例如只剩前几位）。
- 后续 `SPI TX` 看起来与输入不一致，或输出“像乱码”的字节流。

---

## 根因分析（为什么会丢字/缺字）

### 根因 1：在 UART 接收中断回调里做阻塞发送（高概率导致 ORE 溢出丢字节）

典型错误模式：

- `HAL_UART_RxCpltCallback()` 每收到 1 字节，就调用 `HAL_UART_Transmit()` 去回显。
- `HAL_UART_Transmit()` 是**阻塞式**：会占用中断执行时间。
- 在中断阻塞期间，新的字节可能已经到达 UART 接收寄存器/移位寄存器。
- 如果接收没有及时重新 arm（没有及时再次 `HAL_UART_Receive_IT()`），硬件可能产生 **ORE（Overrun Error）**，导致后续字节被丢弃。

结果表现：

- 同一行字符串中“少几个字符”是随机的、非固定位置的（与串口数据到达时机、波特率、中断时延相关）。

### 根因 2：主循环存在长时间阻塞段，帧队列过浅导致“拥塞丢帧”

本工程 `main()` 循环中包含长时间喂狗/延时逻辑 + SPI 阻塞收发：

- 若串口助手短时间内连续发送多行，主循环可能来不及及时取走每一行。
- 若接收侧只用“单缓冲/浅队列”，队列满时就会丢掉旧数据，从而表现为“少一段/少一行”。

---

## 修复方案与实现（本工程最终采用）

### 修复目标

- **中断里尽量短**：只做“取字节 + 拼帧 + 立刻重新开启接收”，避免中断阻塞导致 ORE。
- **用队列缓冲帧**：主循环再慢也不会覆盖上一帧；短时间多行输入也尽量不丢。

### 具体修改点

#### 1) UART 接收改为“行构建缓冲 + 多帧队列”

工程文件：`Src/usart.c`

- 中断回调只做：
  - 把字节加入 `build_line[]`
  - 碰到 `\\r` 或 `\\n` 认为一行结束，将该行入队
  - 立刻再次 `HAL_UART_Receive_IT()` 开启下一字节接收
- 队列深度从 **4 增加到 16**（`RX_FRAME_QUEUE_SIZE`），降低主循环忙时的拥塞丢帧概率。
- 增加了退格支持（0x08/0x7F），避免串口助手本地编辑产生异常字符时影响行缓冲。

#### 2) 从中断中移除“逐字节回显”，改为主循环整行回显

工程文件：`Src/main.c`

- 取到一行后：
  - 复制到 `spi_test_buffer`
  - **整行回显 + `\\r\\n`**
  - 再触发 SPI `TransmitReceive` 并打印 `SPI TX / SPI RX`

这样做的好处：

- 回显不再阻塞中断，显著降低 ORE 溢出导致的丢字。
- 回显与 SPI 发送使用同一份“已经出队的完整帧”，避免“看起来回显一套、SPI 发送另一套”的错位。

---

## 验证建议（如何确认问题已解决）

1) 串口助手设置：
   - 波特率 115200
   - 发送可带 `\\r` 或 `\\r\\n`（两者都支持）
2) 连续快速发送多次不同长度字符串（短/长混合），观察：
   - **整行回显**是否与下发一致
   - `SPI TX:` 是否与回显一致（长度、内容一致）
   - `SPI RX:` 内容取决于从机回传策略（不做回环时出现不可打印字符是可能的，但不应再出现“由于串口丢字导致 TX 变短”的现象）

---

## SPI 收发代码逐行详解（手把手教程）

本章详细解释主MCU的SPI收发代码，适合新手理解每一行的含义和语法。

### 1. 核心收发函数：`SPI2_TransmitThenReceive`

**位置**：`Src/spi.c`（由CubeMX生成的USER CODE区域）

**功能**：先发送命令，等待从机处理，再接收响应（适用于"命令-响应"式通信）

```c
HAL_StatusTypeDef SPI2_TransmitThenReceive(uint8_t *tx_data, uint16_t tx_size,
                                          uint8_t *rx_data, uint16_t rx_size)
{
```

| 代码 | 解释 |
|------|------|
| `HAL_StatusTypeDef` | 函数返回类型，HAL库定义的状态码（HAL_OK表示成功） |
| `SPI2_TransmitThenReceive` | 函数名，自定义命名 |
| `uint8_t *tx_data` | 发送数据指针，uint8_t是8位无符号整数（即字节） |
| `uint16_t tx_size` | 发送数据长度，uint16_t是16位无符号整数 |
| `uint8_t *rx_data` | 接收数据缓冲区指针 |
| `uint16_t rx_size` | 期望接收的数据长度 |

#### 第1步：检查SPI状态并清理

```c
    HAL_StatusTypeDef status = HAL_OK;  // 初始化状态为OK

    // 确保SPI不在忙状态
    if (hspi2.State == HAL_SPI_STATE_BUSY_TX || hspi2.State == HAL_SPI_STATE_BUSY_RX) {
        HAL_SPI_Abort(&hspi2);  // 强制中止SPI，来自HAL库(stm32f1xx_hal_spi.h)
        for (volatile int i = 0; i < 1000; i++) {  // 延时循环
            HAL_WWDG_Refresh(&hwwdg);  // 喂狗，防止看门狗复位
        }
    }
```

**关键点**：
- `hspi2`：全局SPI句柄，由CubeMX生成（在`spi.c`中定义）
- `HAL_SPI_STATE_BUSY_TX/RX`：HAL库定义的SPI状态常量
- `volatile`：告诉编译器不要优化此变量，确保循环真正执行
- `HAL_WWDG_Refresh()`：刷新独立看门狗，来自HAL库(stm32f1xx_hal_wwdg.h)

#### 第2步：清空接收缓冲区

```c
    // 清空接收缓冲区
    if (rx_data != NULL && rx_size > 0) {  // 检查指针非空且长度有效
        memset(rx_data, 0, rx_size);  // 将缓冲区填充为0，来自标准C库(string.h)
    }
```

**为什么清空**：避免旧数据干扰，方便后续判断有效数据长度。

#### 第3步：拉低片选（CS）

```c
    // 选择设备（片选低电平）
    HAL_GPIO_WritePin(SPI2_CS_PORT, SPI2_CS_PIN, GPIO_PIN_RESET);
```

**宏定义来源**（在`spi.c`开头定义）：
```c
#define SPI2_CS_PORT    GPIOB       // CS引脚所在的GPIO端口
#define SPI2_CS_PIN     GPIO_PIN_12 // CS引脚编号
```

- `HAL_GPIO_WritePin()`：HAL库GPIO操作函数(stm32f1xx_hal_gpio.h)
- `GPIO_PIN_RESET`：拉低电平（0V），有效选中从机

#### 第4步：发送命令

```c
    // 先发送命令/数据
    if (tx_data != NULL && tx_size > 0) {  // 检查参数有效性
        status = HAL_SPI_Transmit(&hspi2, tx_data, tx_size, 100);  // HAL库发送函数
        if (status != HAL_OK) {  // 检查发送是否成功
            HAL_GPIO_WritePin(SPI2_CS_PORT, SPI2_CS_PIN, GPIO_PIN_SET);  // 失败则拉高CS
            return status;  // 返回错误状态
        }
    }
```

- `HAL_SPI_Transmit()`：HAL库阻塞式发送函数(stm32f1xx_hal_spi.h)
- 参数含义：(&句柄, 数据指针, 长度, 超时时间ms)

#### 第5步：延时等待从机处理（关键！）

```c
    // 重要：给从MCU足够的时间处理命令并准备响应
    uint32_t delay_target = 50000;  // 默认延时约50ms（循环次数）
    
    // 检查是否是FGRD命令（以FGRD开头）- 不区分大小写
    if (tx_data != NULL && tx_size >= 4) {  // 确保至少有4个字符
        if ((tx_data[0] == 'F' || tx_data[0] == 'f') &&  // 第1字符是F或f
            (tx_data[1] == 'G' || tx_data[1] == 'g') &&  // 第2字符是G或g
            (tx_data[2] == 'R' || tx_data[2] == 'r') &&  // 第3字符是R或r
            (tx_data[3] == 'D' || tx_data[3] == 'd')) {  // 第4字符是D或d
            delay_target = 500000;  // FGRD命令需要约500ms等待FG捕获
        }
    }
    
    // 延时循环，期间喂狗
    for (volatile uint32_t delay_cnt = 0; delay_cnt < delay_target; delay_cnt++) {
        HAL_WWDG_Refresh(&hwwdg);  // 每次循环都喂狗
    }
```

**为什么需要延时**：
- 从MCU收到命令后需要时间解析、执行（如转发到UART、控制外设）
- FGRD命令特殊：需要等待FG（转速反馈）信号捕获完成，约需500ms

#### 第6步：接收响应

```c
    // 接收响应 - 使用全双工传输接收数据
    if (rx_data != NULL && rx_size > 0) {
        uint8_t dummy[32];  // 定义32字节的dummy缓冲区（栈上分配）
        memset(dummy, 0xFF, sizeof(dummy));  // 填充为0xFF

        uint16_t remaining = rx_size;  // 剩余待接收字节数
        uint16_t offset = 0;           // 当前写入位置偏移
        
        // 分块接收，避免栈溢出
        while (remaining > 0) {
            // 计算本次接收的块大小：取剩余长度和缓冲区大小的较小值
            uint16_t chunk = (remaining > (uint16_t)sizeof(dummy)) 
                             ? (uint16_t)sizeof(dummy) 
                             : remaining;
            
            // 全双工收发：发送dummy，同时接收数据
            status = HAL_SPI_TransmitReceive(&hspi2, dummy, &rx_data[offset], chunk, 100);
            if (status != HAL_OK) {  // 出错处理
                HAL_GPIO_WritePin(SPI2_CS_PORT, SPI2_CS_PIN, GPIO_PIN_SET);
                return status;
            }
            remaining -= chunk;  // 更新剩余字节数
            offset += chunk;     // 更新偏移量
        }
    }
```

**SPI全双工原理**：
- 主机必须发送数据才能产生时钟，从而接收数据
- dummy数据（0xFF）只是用来"占位"产生时钟，从机不关心其内容

#### 第7步：拉高片选，结束通信

```c
    // 短暂延时，确保总线空闲
    for (volatile int i = 0; i < 1000; i++) {
        HAL_WWDG_Refresh(&hwwdg);
    }

    // 取消选择设备（片选高电平）
    HAL_GPIO_WritePin(SPI2_CS_PORT, SPI2_CS_PIN, GPIO_PIN_SET);

    return status;  // 返回最终状态
}
```

---

### 2. 命令处理函数：`Process_SPI_Test_Command`

**位置**：`Src/main.c`（由CubeMX生成的USER CODE区域）

**功能**：处理串口输入的命令，通过SPI发送给从机，并打印响应

```c
void Process_SPI_Test_Command(void)
{
    uint16_t payload_len = strlen((char*)spi_test_buffer);  // 计算命令长度
```

- `strlen()`：标准C库函数，计算字符串长度（不含结尾的'\0'）
- `(char*)`：类型转换，将uint8_t指针转为char指针
- `spi_test_buffer`：全局缓冲区，存储从串口接收的命令

#### 参数检查

```c
    if (payload_len == 0) {  // 空命令检查
        HAL_UART_Transmit(&huart1, (uint8_t*)"SPI Error: Empty command\r\n", 26, 100);
        return;  // 提前返回
    }

    if (payload_len > 60) {  // 长度限制检查
        HAL_UART_Transmit(&huart1, (uint8_t*)"SPI Error: Command too long\r\n", 29, 100);
        return;
    }
```

- `HAL_UART_Transmit()`：HAL库串口发送函数(stm32f1xx_hal_uart.h)
- `"\r\n"`：回车换行符，Windows风格的行尾

#### 构造发送帧

```c
    // 构造发送帧：payload + \r\n
    uint8_t tx_frame[64];  // 定义64字节的发送缓冲区
    memset(tx_frame, 0, sizeof(tx_frame));  // 清空缓冲区
    memcpy(tx_frame, spi_test_buffer, payload_len);  // 复制命令内容
    tx_frame[payload_len] = '\r';      // 添加回车符
    tx_frame[payload_len + 1] = '\n';  // 添加换行符
    uint16_t tx_len = payload_len + 2;  // 总长度 = 命令长度 + 2
```

- `memcpy()`：标准C库函数，内存拷贝
- 添加`\r\n`是因为从MCU的UART协议要求以换行符结束

#### 打印发送内容

```c
    HAL_UART_Transmit(&huart1, (uint8_t*)"SPI TX: ", 8, 100);  // 打印前缀
    HAL_UART_Transmit(&huart1, tx_frame, tx_len, 100);         // 打印发送数据
    HAL_UART_Transmit(&huart1, (uint8_t*)"\r\n", 2, 100);      // 打印换行
```

#### 调用SPI收发

```c
    // 接收缓冲区
    uint8_t rx_frame[128];  // 定义128字节的接收缓冲区
    memset(rx_frame, 0, sizeof(rx_frame));  // 清空

    // 调用SPI收发函数
    HAL_StatusTypeDef status = SPI2_TransmitThenReceive(tx_frame, tx_len, rx_frame, 128);
```

#### 解析并打印响应

```c
    if (status == HAL_OK) {  // 通信成功
        // 查找有效响应数据（跳过前导0x00或0xFF）
        uint16_t start_idx = 0;
        while (start_idx < 128 && (rx_frame[start_idx] == 0x00 || rx_frame[start_idx] == 0xFF)) {
            start_idx++;  // 跳过无效字节
        }
        
        // 找到响应的结束位置
        uint16_t resp_len = 0;
        for (uint16_t i = start_idx; i < 128; i++) {
            // 遇到以下字符视为结束：
            // - 0字节（字符串结束符）
            // - 小于0x20的字符（控制字符）
            // - 0xFF（填充字节）
            if (rx_frame[i] == 0 || rx_frame[i] < 0x20 || rx_frame[i] == 0xFF) {
                break;
            }
            resp_len++;
        }
        
        // 打印响应
        HAL_UART_Transmit(&huart1, (uint8_t*)"SPI RX: ", 8, 100);
        if (resp_len > 0) {
            HAL_UART_Transmit(&huart1, &rx_frame[start_idx], resp_len, 100);
        } else {
            HAL_UART_Transmit(&huart1, (uint8_t*)"(empty)", 7, 100);
        }
        HAL_UART_Transmit(&huart1, (uint8_t*)"\r\n", 2, 100);
    } else {  // 通信失败
        HAL_UART_Transmit(&huart1, (uint8_t*)"SPI Error: Communication failed\r\n", 33, 100);
    }
}
```

---

## 遇到的问题与解决办法

### 问题1：主MCU收到全0x55（'U'字符）

**现象**：主MCU打印`SPI RX: UUUUU...`，无法收到正确响应

**原因分析**：
- 从MCU的SPI状态机设计有问题，一直停留在ACK状态
- 从MCU收到每个字节都回复0x55（ACK），没有进入响应发送状态

**解决办法**：重新设计从MCU的SPI状态机
- 简化状态：IDLE → CMD_RX → CMD_READY → RESP_READY → RESP_SENDING → RESP_SENT
- 收到完整命令（以\r\n结尾）后，等待主程序处理并设置响应
- 响应准备好后，在TXE中断中发送响应数据而非ACK

### 问题2：FGRD命令需要发送两次才能收到正确结果

**现象**：第一次返回empty，第二次才收到上一次的转速信息

**原因分析**：
- 从MCU处理FGRD命令需要时间捕获FG信号（约500ms）
- 主MCU发送命令后立即读取响应，此时从MCU还没准备好
- 从MCU的响应被延迟到下一次SPI通信才发出

**解决办法**：主MCU增加延时等待
```c
// 检查是否是FGRD命令，如果是则增加延时
if ((tx_data[0] == 'F' || tx_data[0] == 'f') &&
    (tx_data[1] == 'G' || tx_data[1] == 'g') &&
    (tx_data[2] == 'R' || tx_data[2] == 'r') &&
    (tx_data[3] == 'D' || tx_data[3] == 'd')) {
    delay_target = 500000;  // FGRD命令延时约500ms
}
```

### 问题3：响应数据前有无效字节（0x00或0xFF）

**现象**：SPI RX打印包含前导空字符或乱码

**原因分析**：
- SPI通信开始时，从机可能还未准备好，发送了填充字节
- 全双工通信中，主机发送dummy字节时，从机回复了无效数据

**解决办法**：主MCU解析响应时跳过前导无效字节
```c
// 查找有效响应数据（跳过前导0x00或0xFF）
uint16_t start_idx = 0;
while (start_idx < 128 && (rx_frame[start_idx] == 0x00 || rx_frame[start_idx] == 0xFF)) {
    start_idx++;
}
// 从start_idx开始打印有效数据
```

### 问题4：FGRD命令返回两次响应

**现象**：从MCU先返回`[OK] FGRD pending capture`，再返回实际转速信息

**原因分析**：
- 从MCU采用异步处理：立即返回ACK，稍后打印结果
- 但SPI通信需要同步响应

**解决办法**：修改从MCU为同步处理
- 收到FGRD命令后，等待捕获完成（最多2秒超时）
- 直接返回包含转速信息的完整响应
- 不再分两次返回

---

## FGRD命令特殊处理说明

### 什么是FGRD

FGRD = **F**an **G**o **R**ea**D**（风扇转速读取）
- 格式：`FGRD<EN0><EN1><A0><A1><A2>`
- 示例：`FGRD1000` 表示EN0=1, EN1=0, A0=0, A1=0, A2=0

### 为什么FGRD需要特殊延时

1. **MUX切换延时**：模拟开关切换通道后需要约20ms稳定
2. **FG捕获时间**：需要检测至少2个边沿才能计算频率，取决于风扇转速
   - 1000 RPM的风扇，约需60ms捕获2个边沿
   - 加上余量，设计为500ms延时

### Captures: 2的含义

表示TIM2输入捕获中断触发了2次：
- FG信号每转产生2个脉冲（风扇特性）
- 捕获2个边沿（上升沿+下降沿）即可计算一个完整周期
- 如果Captures < 2，表示捕获未完成，频率显示为0

---

## 补充说明：SPI RX "看似乱码"不一定是异常

如果从机没有实现"回显/回环"协议，主机侧 `HAL_SPI_TransmitReceive()` 收到的字节可能是：

- 从机状态机的其他数据
- 0xFF/0x00 等填充值
- 未定义/不可打印字节

因此 **`SPI RX` 显示成 `����` 或者不可见字符**并不必然代表 SPI 有问题；判断 SPI 是否正确应结合：

- 示波器/逻辑分析仪观察 CS/SCK/MOSI/MISO 时序
- 从机侧协议定义（是否应该回传与 TX 相同的数据）


# STM32 UART 串口通信功能讲解

本文档详细介绍STM32微控制器的UART串口通信功能，包括基本原理、代码实现和使用方法，适合没有嵌入式开发基础的初学者阅读。

## 一、UART 基本原理

### 1. 什么是 UART？

UART（Universal Asynchronous Receiver Transmitter）是一种通用异步收发传输器，是微控制器中最常用的通信接口之一。

**简单来说**：UART 就是一种让设备之间通过数据线进行文字、数字等信息传递的方式。就像两个人通过电话线说话一样，一方说，另一方听。

### 2. UART 通信的基本要素

| 要素 | 说明 | 我们的设置 |
|------|------|------------|
| 波特率 | 数据传输速度，单位是 bps（每秒位数） | 115200 bps |
| 数据位 | 每个字符占用的位数 | 8 位（一个字节） |
| 停止位 | 每个字符结束时的标记 | 1 位 |
| 校验位 | 用于检测传输错误 | 无 |

### 3. 通信过程

1. **发送数据**：发送方将字符转换为二进制数据，按照设定的波特率一位一位地发送
2. **接收数据**：接收方按照相同的波特率一位一位地接收数据，再转换回字符
3. **数据格式**：每个字符由起始位、数据位、（可选的）校验位和停止位组成

### 4. 硬件连接

在我们的项目中：
- **PA9**：USART1 的发送引脚（TX）
- **PA10**：USART1 的接收引脚（RX）

## 二、代码实现

### 1. 主要文件

| 文件 | 功能 |
|------|------|
| `Src/usart.c` | UART 初始化和中断处理 |
| `Inc/usart.h` | UART 相关函数声明 |
| `Src/main.c` | 主程序逻辑 |

### 2. 代码改动

#### 2.1 增加接收缓冲区和中断回调函数

在 `Src/usart.c` 文件中添加了以下功能：

```c
// 串口接收缓冲区定义
#define RX_BUFFER_SIZE 100
uint8_t rx_buffer[RX_BUFFER_SIZE];
volatile uint8_t rx_index = 0;
static volatile uint8_t rx_ready = 0;

/**
  * @brief  串口接收完成回调函数
  * @param  huart: UART句柄
  * @retval None
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == USART1)
  {
    // 检查是否收到回车
    if (rx_buffer[rx_index] == '\r')
    {
      // 确保至少有一个有效字符
      if (rx_index > 0)
      {
        rx_buffer[rx_index] = '\0';  // 添加字符串结束符
        rx_ready = 1;
      }
      // 重置索引，准备下一次接收
      rx_index = 0;
      HAL_UART_Receive_IT(&huart1, &rx_buffer[rx_index], 1);
    }
    else if (rx_index >= RX_BUFFER_SIZE - 1)
    {
      // 缓冲区已满，强制结束接收
      rx_buffer[rx_index] = '\0';
      rx_ready = 1;
      // 重置索引，准备下一次接收
      rx_index = 0;
      HAL_UART_Receive_IT(&huart1, &rx_buffer[rx_index], 1);
    }
    else
    {
      rx_index++;
      // 继续接收下一个字节
      HAL_UART_Receive_IT(&huart1, &rx_buffer[rx_index], 1);
    }
  }
}

/**
  * @brief  获取接收到的字符串
  * @param  buffer: 输出缓冲区
  * @param  size: 缓冲区大小
  * @retval 接收的字节数
  */
uint8_t USART1_GetReceivedData(uint8_t *buffer, uint8_t size)
{
  if (!rx_ready) return 0;
  
  uint8_t len = 0;
  // 计算实际接收到的字节数（不包括结束符）
  while (len < RX_BUFFER_SIZE && rx_buffer[len] != '\0') {
    len++;
  }
  
  // 确保至少有一个有效字符
  if (len > 0)
  {
    if (len > size - 1) len = size - 1;
    
    memcpy(buffer, rx_buffer, len);
    buffer[len] = '\0';
    
    // 重置接收状态
    rx_ready = 0;
    
    return len;
  }
  
  // 如果没有有效字符，重置接收状态
  rx_ready = 0;
  return 0;
}
```

#### 2.2 在头文件中添加函数声明

在 `Inc/usart.h` 文件中添加了以下声明：

```c
#define RX_BUFFER_SIZE 100
extern uint8_t rx_buffer[RX_BUFFER_SIZE];
extern volatile uint8_t rx_index;

/**
  * @brief  获取接收到的字符串
  * @param  buffer: 输出缓冲区
  * @param  size: 缓冲区大小
  * @retval 接收的字节数
  */
uint8_t USART1_GetReceivedData(uint8_t *buffer, uint8_t size);
```

#### 2.3 修改主程序逻辑

在 `Src/main.c` 文件中修改了接收逻辑：

```c
/* 启动UART中断接收 */
rx_index = 0;
HAL_UART_Receive_IT(&huart1, &rx_buffer[rx_index], 1);

/* 在主循环中 */
/* 检查串口接收 - 处理接收到的字符串 */
uint8_t rx_data[10];
uint8_t len = USART1_GetReceivedData(rx_data, sizeof(rx_data));
if (len > 0) {
  /* 回显接收到的字符串 */
  HAL_UART_Transmit(&huart1, rx_data, len, 10);
  
  /* 发送换行 */
  const char newline[] = "\r\n";
  HAL_UART_Transmit(&huart1, (uint8_t*)newline, 2, 10);
}
```

## 三、为什么这样修改

### 1. 原代码的问题

原代码使用阻塞式接收方式，存在以下问题：

- **数据丢失**：当发送多个字符时，只能收到前1-2个字符
- **响应缓慢**：接收数据时会阻塞主程序运行
- **使用不便**：需要手动处理每个字符的接收和处理

### 2. 修改后的优势

1. **中断接收**：使用中断方式接收数据，不会阻塞主程序
2. **完整接收**：能够接收完整的字符串，不会丢失数据
3. **自动处理**：接收完成后会自动通知主程序
4. **使用简单**：只需要调用一个函数就能获取完整的字符串

### 3. 关键技术点

#### 3.1 中断接收模式

**中断**就像手机的消息通知，当有新消息时会提醒你，而不需要你一直盯着手机看。在UART中，当接收到数据时，会触发一个"通知"（中断），告诉微控制器有数据来了。

#### 3.2 缓冲区

**缓冲区**就像一个临时存储盒，当数据一个接一个地到来时，会先存放在这个盒子里，等所有数据都到齐了，再一起处理。

#### 3.3 接收完成判断

当收到回车符（\r）时，认为一个完整的字符串已经发送完毕，就可以处理这个字符串了。

## 四、使用方法

### 1. 硬件连接

将 USB-to-TTL 转换器按照以下方式连接到 STM32 开发板：

| USB-to-TTL | STM32 开发板 |
|------------|--------------|
| TXD | PA10 (USART1_RX) |
| RXD | PA9 (USART1_TX) |
| GND | GND |
| VCC | 3.3V |

### 2. 软件设置

1. **打开串口助手软件**（如友善串口调试助手）
2. **配置串口参数**：
   - 端口：选择对应的 COM 口
   - 波特率：115200
   - 数据位：8
   - 停止位：1
   - 校验位：无
3. **勾选"自动换行"** 选项

### 3. 发送和接收数据

1. **发送数据**：在发送框中输入要发送的文字（如"Hello World"），然后点击"发送"按钮
2. **接收数据**：在接收窗口中可以看到 STM32 发回的相同数据

**示例**：
- 你发送：`Hello World`
- 你会收到：`Hello World`（可能显示为两行，这是正常现象）

### 4. 常见问题及解决方法

| 问题 | 可能原因 | 解决方法 |
|------|----------|----------|
| 收不到数据 | 波特率设置错误 | 确保波特率设置为 115200 |
|  | 硬件连接错误 | 检查 TXD 和 RXD 是否接反 |
| 只收到部分数据 | 缓冲区大小限制 | 发送的字符串不要太长（建议不超过 90 个字符） |
| 显示乱码 | 波特率不匹配 | 确保两端波特率一致 |

## 五、代码使用说明

### 1. 初始化 UART

系统启动时会自动初始化 UART，不需要手动操作：

```c
// 系统自动执行的初始化代码
MX_USART1_UART_Init();
```

### 2. 发送数据

要发送数据，可以使用 `HAL_UART_Transmit` 函数：

```c
const char *message = "Hello, UART!";
HAL_UART_Transmit(&huart1, (uint8_t*)message, strlen(message), HAL_MAX_DELAY);
```

### 3. 接收数据

系统会自动接收数据，你只需要在主循环中检查是否有数据接收完成：

```c
uint8_t rx_data[50];  // 接收缓冲区
uint8_t len = USART1_GetReceivedData(rx_data, sizeof(rx_data));
if (len > 0) {
  // 有数据接收完成，可以处理这些数据
  // 例如：打印到串口
  HAL_UART_Transmit(&huart1, rx_data, len, 100);
}
```

## 六、进阶应用

### 1. 发送数字

要发送数字，需要先将数字转换为字符串：

```c
int number = 12345;
char buffer[20];
sprintf(buffer, "The number is: %d\r\n", number);
HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), 100);
```

### 2. 发送传感器数据

如果连接了传感器，可以将传感器数据发送到电脑：

```c
// 假设 sensor_value 是从传感器读取的值
float sensor_value = 25.5;
char buffer[30];
sprintf(buffer, "Temperature: %.1f°C\r\n", sensor_value);
HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), 100);
```

### 3. 响应特定命令

可以根据接收到的命令执行不同的操作：

```c
uint8_t rx_data[50];
uint8_t len = USART1_GetReceivedData(rx_data, sizeof(rx_data));
if (len > 0) {
  // 比较接收到的命令
  if (strcmp((char*)rx_data, "LED_ON") == 0) {
    // 打开 LED
    HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
    HAL_UART_Transmit(&huart1, (uint8_t*)"LED turned on\r\n", 13, 100);
  } else if (strcmp((char*)rx_data, "LED_OFF") == 0) {
    // 关闭 LED
    HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
    HAL_UART_Transmit(&huart1, (uint8_t*)"LED turned off\r\n", 14, 100);
  }
}
```

## 七、总结

UART 串口通信是嵌入式开发中最基础、最常用的通信方式之一。通过本文的介绍，你应该已经了解：

1. **UART 的基本原理**：就像两个人通过电话线交谈
2. **代码实现**：使用中断方式接收数据，避免数据丢失
3. **使用方法**：如何连接硬件、配置软件、发送和接收数据
4. **进阶应用**：如何发送数字、传感器数据和响应命令

希望本文能帮助你快速上手 STM32 的 UART 串口通信功能！

## 附：完整代码文件

### 1. `Src/usart.c` 主要修改部分

```c
// 串口接收缓冲区定义
#define RX_BUFFER_SIZE 100
uint8_t rx_buffer[RX_BUFFER_SIZE];
volatile uint8_t rx_index = 0;
static volatile uint8_t rx_ready = 0;

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == USART1)
  {
    if (rx_buffer[rx_index] == '\r')
    {
      if (rx_index > 0)
      {
        rx_buffer[rx_index] = '\0';
        rx_ready = 1;
      }
      rx_index = 0;
      HAL_UART_Receive_IT(&huart1, &rx_buffer[rx_index], 1);
    }
    else if (rx_index >= RX_BUFFER_SIZE - 1)
    {
      rx_buffer[rx_index] = '\0';
      rx_ready = 1;
      rx_index = 0;
      HAL_UART_Receive_IT(&huart1, &rx_buffer[rx_index], 1);
    }
    else
    {
      rx_index++;
      HAL_UART_Receive_IT(&huart1, &rx_buffer[rx_index], 1);
    }
  }
}

uint8_t USART1_GetReceivedData(uint8_t *buffer, uint8_t size)
{
  if (!rx_ready) return 0;
  
  uint8_t len = 0;
  while (len < RX_BUFFER_SIZE && rx_buffer[len] != '\0') {
    len++;
  }
  
  if (len > 0)
  {
    if (len > size - 1) len = size - 1;
    memcpy(buffer, rx_buffer, len);
    buffer[len] = '\0';
    rx_ready = 0;
    return len;
  }
  
  rx_ready = 0;
  return 0;
}
```

### 2. `Src/main.c` 主要修改部分

```c
/* 启动UART中断接收 */
rx_index = 0;
HAL_UART_Receive_IT(&huart1, &rx_buffer[rx_index], 1);

/* 在主循环中 */
uint8_t rx_data[10];
uint8_t len = USART1_GetReceivedData(rx_data, sizeof(rx_data));
if (len > 0) {
  HAL_UART_Transmit(&huart1, rx_data, len, 10);
  const char newline[] = "\r\n";
  HAL_UART_Transmit(&huart1, (uint8_t*)newline, 2, 10);
}
```

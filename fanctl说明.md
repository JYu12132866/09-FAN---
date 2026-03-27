# 风扇控制模块说明

## 概述

本文档对比分析了两个工程中的风扇控制实现：
- Simu_STM32F407_UDP_FAN_Temp 工程：采用模块化设计，支持温度自动控制
- 08 FAN_CTRL 工程：风扇控制代码直接写在 main.c 中，仅支持手动控制

---

## Simu_STM32F407_UDP_FAN_Temp 工程分析

### 模块结构

- **fanctl.c**：风扇控制核心实现
- **fanctl.h**：头文件，定义结构体和函数声明

---

### fanctl.c 逐行代码解析

#### 1. 头文件包含部分（第1-6行）

```c
#include "fanctl.h"      // 包含风扇控制头文件，定义结构体和函数声明
#include <stdint.h>      // 包含标准整数类型定义（uint8_t, uint16_t等）
#include "tim.h"         // 包含定时器头文件，使用HAL库定时器功能
#include "logger.h"      // 包含日志头文件，用于输出调试信息
#include "modectl.h"     // 包含模式控制头文件，获取工作模式
// #include "err.h"       // 错误处理头文件（已注释掉）
```

**语法说明**：
- `#include` 是C语言预处理指令，用于包含头文件
- `"xxx.h"` 表示从当前项目目录查找头文件
- `<xxx.h>` 表示从系统标准库路径查找头文件
- `//` 表示单行注释

---

#### 2. 宏定义部分（第8-11行）

```c
#define FAN_PWM_FLOOR 40       // 风扇PWM最小值（40%），防止风扇停转
#define FAN_PWM_CEIL  100      // 风扇PWM最大值（100%），满速运行
#define FAN_PWM_FIX_NORMAL  70 // 正常工作模式固定PWM值（70%）
#define FAN_PWM_FIX_HI_ALT  100 // 高海拔模式固定PWM值（100%）
```

**语法说明**：
- `#define` 是宏定义指令，用于定义常量
- 宏定义在编译时进行文本替换，不占内存
- 命名规范：通常使用大写字母和下划线

**功能说明**：
- `FAN_PWM_FLOOR`：设置风扇最低转速为40%，防止风扇停转导致散热不足
- `FAN_PWM_CEIL`：设置风扇最高转速为100%
- `FAN_PWM_FIX_NORMAL`：正常模式下固定70%转速
- `FAN_PWM_FIX_HI_ALT`：高海拔模式下固定100%转速（空气稀薄需要更高转速）

---

#### 3. 风扇硬件映射表注释（第13-26行）

```c
/* 风扇timer与硬件映射关系表
| Fan  | timer | channel     | 说明
   1     htim1   TIM_CHANNEL_1   FAN1/FAN5
   2     htim1   TIM_CHANNEL_2   no use
   ...
*/
```

**功能说明**：
- 这个注释表格说明了10个风扇分别使用哪个定时器和通道
- 用于硬件设计和软件配置的对应关系

---

#### 4. 风扇位置示意图注释（第28-45行）

```c
/* 风扇位置示意图
a. 散热风扇1 
b. 散热风扇2
c. 激光散热器1
...
*/
```

**功能说明**：
- 用ASCII图形展示风扇在设备中的物理位置
- 帮助理解风扇布局和维护

---

#### 5. 风扇信息数组定义（第49-60行）

```c
/* 各风扇通道timer/channel映射关系 */
FAN_INFO fans_info[] = {
    {&htim1, TIM_CHANNEL_1},    //Fan_1
    {&htim1, TIM_CHANNEL_2},    //Fan_2
    {&htim2, TIM_CHANNEL_1},    //Fan_3
    {&htim2, TIM_CHANNEL_2},    //Fan_4
    {&htim4, TIM_CHANNEL_4},    //Fan_5  
    {&htim3, TIM_CHANNEL_1},    //Fan_6
    {&htim3, TIM_CHANNEL_2},    //Fan_7
    {&htim1, TIM_CHANNEL_3},    //Fan_8
    {&htim1, TIM_CHANNEL_4},    //Fan_9
    {&htim4, TIM_CHANNEL_2},    //Fan_10
};
```

**语法说明**：
- `FAN_INFO` 是在 fanctl.h 中定义的结构体类型
- `fans_info[]` 是结构体数组，存储10个风扇的硬件信息
- `{&htim1, TIM_CHANNEL_1}` 是结构体初始化列表
- `&htim1` 是定时器句柄的地址（指针）
- `TIM_CHANNEL_1` 是定时器通道宏定义

**数据结构（来自fanctl.h）**：
```c
typedef struct 
{
    TIM_HandleTypeDef*  pwm_tim_hdl;  // 定时器句柄指针
    uint32_t            pwm_tim_ch;   // 定时器通道
}FAN_INFO;
```

**功能说明**：
- 定义了10个风扇的硬件配置信息
- 每个风扇对应一个定时器和一个通道
- 通过数组索引（0-9）可以访问任意风扇的硬件信息

---

#### 6. 温度控制曲线参数定义（第63-89行）

```c
/* 温度控制曲线参数说明
    y = kx + b 线性关系，根据温度计算PWM占空比
    x: 温度
    y: 目标PWM占空比
分段拐点：
    x: 20.00, 40.00, 60.00, 80.00
    y: 30.00, 40.00, 60.00, 100.00
*/

// 温度控制曲线1定义
CURVE_PARAM curve1 = {
    .section_num = 5,  // 曲线分为5段
    .section_param[0] = {-99.00, 30.00 ,0.00, FAN_PWM_FLOOR},  // 第1段
    .section_param[1] = {30.00, 40.00, 1.00, 10.00},           // 第2段
    .section_param[2] = {40.00, 60.00, 1.00, 10.00},           // 第3段
    .section_param[3] = {60.00, 80.00, 1.50, -20.00},          // 第4段
    .section_param[4] = {80.00, 199.00, 0.00, FAN_PWM_CEIL},   // 第5段
};
```

**语法说明**：
- `.section_num = 5` 是C99标准的指定初始化器语法
- 可以直接指定结构体成员的初始值
- 提高了代码可读性

**数据结构（来自fanctl.h）**：
```c
typedef struct 
{
    int section_num;    // 曲线分段数量
    CURVE_SECTION_PARAM section_param[5]; // 每段参数
}CURVE_PARAM;

typedef struct 
{
    float floor_temp;   // 温度下限
    float ceil_temp;    // 温度上限
    float k;            // 斜率
    float b;            // 截距
}CURVE_SECTION_PARAM;
```

**数学原理**：
- 每段使用线性方程 `y = kx + b`
- y 是PWM占空比，x 是温度
- 根据温度所在区间，选择对应的k和b计算PWM值

**curve1各段含义**：
| 段 | 温度范围 | 公式 | PWM范围 |
|---|---------|------|--------|
| 0 | -99°C ~ 30°C | y = 40 | 固定40% |
| 1 | 30°C ~ 40°C | y = x + 10 | 40% ~ 50% |
| 2 | 40°C ~ 60°C | y = x + 10 | 50% ~ 70% |
| 3 | 60°C ~ 80°C | y = 1.5x - 20 | 70% ~ 100% |
| 4 | 80°C ~ 199°C | y = 100 | 固定100% |

---

#### 7. 温度控制曲线2定义（第82-89行）

```c
CURVE_PARAM curve2 = {
    .section_num = 5,
    .section_param[0] = {-99.00, 30.00 ,0.00, FAN_PWM_FLOOR + 10},  // y=50
    .section_param[1] = {30.00, 40.00, 1.00, 20.00},                // y=x+20
    .section_param[2] = {40.00, 60.00, 1.00, 20.00},                // y=x+20
    .section_param[3] = {60.00, 80.00, 1.50, -10.00},               // y=1.5x-10
    .section_param[4] = {80.00, 199.00, 0.00, FAN_PWM_CEIL},        // y=100
};
```

**说明**：
- curve2 比 curve1 整体偏移+10%
- 用于不同的散热需求场景

---

#### 8. 温度曲线指针数组（第92-95行）

```c
CURVE_PARAM* curve_tbl[] = {
    &curve1,      // 曲线1指针
    &curve2,      // 曲线2指针
};
```

**语法说明**：
- `CURVE_PARAM*` 是指向结构体的指针类型
- `&curve1` 取curve1的地址
- 通过指针数组可以动态选择使用哪条曲线

**功能说明**：
- 将多条曲线组织成数组，便于通过索引选择
- 例如：`curve_tbl[0]` 指向 curve1

---

#### 9. 风扇组绑定关系表（第98-101行）

```c
/* 温度控制曲线-温度传感器-风扇绑定关系表 */
FAN_GROUP fan_grp_tbl[] = {
    {1,   4, {2,3,4,6},    3, {0,8,7}},  //组0
    {0,   2, {0, 1, },     2, {5,6}},    //组1
};
```

**数据结构（来自fanctl.h）**：
```c
typedef struct  
{
    int curve_idx;              // 曲线索引
    int temp_sensor_num;        // 温度传感器数量
    int temp_sensor_ch[6];      // 温度传感器通道数组
    int fan_num;                // 风扇数量
    int fan_idx[4];             // 风扇索引数组
}FAN_GROUP;
```

**功能说明**：
- **组0**：使用curve2，监控4个温度传感器（2,3,4,6），控制3个风扇（0,8,7）
- **组1**：使用curve1，监控2个温度传感器（0,1），控制2个风扇（5,6）
- 实现分组管理，不同区域使用不同的温度控制策略

---

#### 10. 全局变量定义（第104-106行）

```c
int  fan_dbg_mode = 0;  // debug标志，为1时进入debug模式，关闭自动控制
float fan_rpm_offset = FAN_OFFSET_NORMAL;  // PWM偏移量
```

**功能说明**：
- `fan_dbg_mode`：调试模式标志
  - 0：正常自动模式
  - 1：调试模式，关闭自动温度控制
- `fan_rpm_offset`：PWM偏移量
  - `FAN_OFFSET_NORMAL` (0)：正常模式
  - `FAN_OFFSET_ECO` (-15)：节能模式，降低15%转速
  - `FAN_OFFSET_HI_ALT` (15)：高海拔模式，增加15%转速

---

#### 11. calc_fan_pwm 函数（第114-134行）

```c
/**
 * @brief 通过温度计算目标PWM值
 * @param max_temp 最高温度
 * @param curve 温度曲线参数
 * @return int 计算得到的PWM值
 */
inline int calc_fan_pwm(float max_temp, CURVE_PARAM* curve)
{
    int curve_section_num = curve->section_num;  // 获取曲线段数
    int fan_pwm = 0;
    
    // 遍历所有温度段，找到对应的区间
    for (size_t i = 0; i < curve_section_num; i++)
    {
        // 检查温度是否在当前段范围内
        if(max_temp >= curve->section_param[i].floor_temp && 
           max_temp < curve->section_param[i].ceil_temp)
        {
            // 使用线性公式 y = kx + b 计算PWM值
            fan_pwm = (curve->section_param[i].k * max_temp + 
                      curve->section_param[i].b + fan_rpm_offset);
            break;  // 找到对应区间后跳出循环
        }
    }
    
    // 限制PWM在有效范围内
    if(fan_pwm < FAN_PWM_FLOOR)
        fan_pwm = FAN_PWM_FLOOR;
    else if(fan_pwm > FAN_PWM_CEIL)
        fan_pwm = FAN_PWM_CEIL;
    
    // 转换为实际PWM寄存器值
    fan_pwm = Tim_PWM_DC_end * fan_pwm / 100;
    return fan_pwm;
}
```

**语法说明**：
- `inline` 关键字建议编译器将函数内联展开，减少调用开销
- `CURVE_PARAM* curve` 是指向结构体的指针参数
- `curve->section_num` 使用箭头运算符访问指针指向的结构体成员
- `size_t` 是无符号整数类型，通常用于数组索引

**函数逻辑**：
1. 获取曲线的段数
2. 遍历每段，找到温度所在的区间
3. 使用公式 `PWM = k * 温度 + b + 偏移量` 计算
4. 限制PWM在最小值和最大值之间
5. 转换为定时器寄存器值

---

#### 12. fan_pwm_auto_set 函数（第137-178行）

```c
void fan_pwm_auto_set(volatile float* temp_array, int temp_num)
{
    // 如果处于调试模式，不执行自动控制
    if(fan_dbg_mode)
        return;

#if(Projector_Model == Non_Model)
    // 自动温度控制模式
    log_info("Auto fan ctl:0x%x", work_mode);
    
    // 计算风扇组数量
    const static int group_num = sizeof(fan_grp_tbl)/sizeof(FAN_GROUP);
    
    // 遍历每个风扇组
    for (size_t i = 0; i < group_num; i++)
    {
        float max_temp = 0;
        int fan_pwm = 0;
        
        // 在组内查找最高温度
        for (size_t j = 0; j < fan_grp_tbl[i].temp_sensor_num; j++)
        {   
            int temp_idx = fan_grp_tbl[i].temp_sensor_ch[j]; 
            if(temp_array[temp_idx] > max_temp)
                max_temp = temp_array[temp_idx];
        }
        
        // 获取对应曲线参数，计算PWM
        CURVE_PARAM* curve = curve_tbl[fan_grp_tbl[i].curve_idx];
        fan_pwm = calc_fan_pwm(max_temp, curve);
        
        // 设置组内所有风扇的PWM
        for (size_t j = 0; j < fan_grp_tbl[i].fan_num; j++)
        {
            int fan_idx = fan_grp_tbl[i].fan_idx[j];
            __HAL_TIM_SET_COMPARE(fans_info[fan_idx].pwm_tim_hdl, 
                                 fans_info[fan_idx].pwm_tim_ch, fan_pwm); 
        }
        
        log_info("Fan group %d Max_temp: %.2f, Fan_pwm %.2f%%", 
                 i, max_temp, (float)(fan_pwm * 100 / Tim_PWM_DC_end));
    }
#else
    // 固定转速模式（非自动模式）
    log_info("Fixed fan ctl:0x%x", work_mode);
    uint16_t fan_pwm = 0;
    
    if(work_mode == WORK_MODE_HI_ALT)
        fan_pwm = Tim_PWM_DC_end * FAN_PWM_FIX_HI_ALT / 100;
    else
        fan_pwm = Tim_PWM_DC_end * FAN_PWM_FIX_NORMAL / 100;

    FAN_PWM_Set(fan_pwm);
#endif
}
```

**语法说明**：
- `volatile float* temp_array`：volatile关键字告诉编译器该变量可能被外部修改
- `sizeof(fan_grp_tbl)/sizeof(FAN_GROUP)`：计算数组元素个数
- `const static`：常量静态变量，只初始化一次
- `#if #else #endif`：条件编译指令

**函数逻辑**：
1. 检查是否处于调试模式，如果是则直接返回
2. 计算风扇组数量
3. 对每个风扇组：
   - 获取组内所有温度传感器的最高温度
   - 根据组配置的曲线计算PWM值
   - 将PWM值设置到组内所有风扇
4. 记录日志信息

**HAL库函数**：
- `__HAL_TIM_SET_COMPARE(htim, Channel, Compare)`：设置定时器比较值，从而改变PWM占空比

---

#### 13. update_fanctl_offset 函数（第180-183行）

```c
inline void update_fanctl_offset(int offset)
{
    fan_rpm_offset = offset;
}
```

**功能说明**：
- 设置风扇转速偏移量
- 用于切换工作模式时调整整体转速
- 例如：节能模式(-15%)、高海拔模式(+15%)

---

#### 14. fan_pwm_set_single 函数（第186-189行）

```c
void fan_pwm_set_single(uint16_t FAN_PWM, int fan_idx)
{
    __HAL_TIM_SET_COMPARE(fans_info[fan_idx].pwm_tim_hdl, 
                         fans_info[fan_idx].pwm_tim_ch, FAN_PWM); 
}
```

**功能说明**：
- 设置单个风扇的PWM值
- 用于调试或手动控制单个风扇
- `fan_idx`：风扇索引（0-9）

---

#### 15. fan_enter_dbg_mode 函数（第191-195行）

```c
inline void fan_enter_dbg_mode(void)
{
    fan_dbg_mode = 1;
    log_info("FanCtl Enter debug mode");
}
```

**功能说明**：
- 进入调试模式
- 设置调试标志为1
- 关闭自动温度控制
- 记录日志

---

#### 16. fan_exit_dbg_mode 函数（第197-201行）

```c
inline void fan_exit_dbg_mode(void)
{
    fan_dbg_mode = 0;
    log_info("FanCtl Exit debug mode");
}
```

**功能说明**：
- 退出调试模式
- 设置调试标志为0
- 恢复自动温度控制
- 记录日志

---

### 核心结构体总结

#### FAN_INFO 结构体
```c
typedef struct 
{
    TIM_HandleTypeDef*  pwm_tim_hdl;  // 定时器句柄指针
    uint32_t            pwm_tim_ch;   // 定时器通道
}FAN_INFO;
```
**用途**：存储单个风扇的硬件配置信息

#### CURVE_SECTION_PARAM 结构体
```c
typedef struct 
{
    float floor_temp;   // 温度下限
    float ceil_temp;    // 温度上限
    float k;            // 斜率
    float b;            // 截距
}CURVE_SECTION_PARAM;
```
**用途**：定义温度曲线的单个分段参数

#### CURVE_PARAM 结构体
```c
typedef struct 
{
    int section_num;                        // 分段数量
    CURVE_SECTION_PARAM section_param[5];   // 分段参数数组
}CURVE_PARAM;
```
**用途**：定义完整的温度控制曲线（最多5段）

#### FAN_GROUP 结构体
```c
typedef struct  
{
    int curve_idx;              // 曲线索引
    int temp_sensor_num;        // 温度传感器数量
    int temp_sensor_ch[6];      // 温度传感器通道数组
    int fan_num;                // 风扇数量
    int fan_idx[4];             // 风扇索引数组
}FAN_GROUP;
```
**用途**：定义风扇组，绑定温度传感器和控制曲线

---

### 调用流程

#### 初始化流程
1. 系统启动时，全局变量 `fan_dbg_mode = 0`，`fan_rpm_offset = FAN_OFFSET_NORMAL`
2. 风扇信息数组 `fans_info` 已静态初始化
3. 温度曲线 `curve1`、`curve2` 已静态初始化
4. 风扇组表 `fan_grp_tbl` 已静态初始化

#### 自动控制流程
1. 在 `main.c` 的 `loop_task_1000ms` 函数中每5秒调用 `fan_pwm_auto_set()`
2. `fan_pwm_auto_set()` 获取温度数组
3. 对每个风扇组：
   - 查找组内最高温度
   - 调用 `calc_fan_pwm()` 计算PWM值
   - 使用 `__HAL_TIM_SET_COMPARE()` 设置PWM
4. 记录日志

#### 调试模式流程
1. 调用 `fan_enter_dbg_mode()` 进入调试模式
2. `fan_pwm_auto_set()` 检测到 `fan_dbg_mode = 1`，直接返回
3. 可以手动调用 `fan_pwm_set_single()` 控制单个风扇
4. 调用 `fan_exit_dbg_mode()` 退出调试模式

---

## 08 FAN_CTRL 工程分析

### 实现方式

风扇控制代码直接写在 `main.c` 中，使用 TIM3 的四个通道控制风扇：
- TIM3_CH1：PUMP_PWM
- TIM3_CH2：FAN_PWM1
- TIM3_CH3：FAN_PWM2
- TIM3_CH4：FAN_PWM3

### 核心函数

1. **PWM_SetDutyPercent_ById**：设置单路PWM占空比
   - 输入：PWM通道ID和占空比
   - 功能：根据占空比计算并设置PWM值

2. **parse_xpwm_cmd**：解析风扇控制命令
   - 输入：命令字符串
   - 功能：解析 xPWMxxx 格式的命令，提取通道ID和关断百分比

### 调用方式

通过 SPI 或 UART 接收命令来控制风扇：

```c
/* xPWMxxx 指令 */
else if (parse_xpwm_cmd((const char *)spi_cmd, &pwm_id, &off_percent)) {
    uint8_t duty_on_percent = (uint8_t)(100U - off_percent);
    PWM_SetDutyPercent_ById(pwm_id, duty_on_percent);
    snprintf((char *)response_buf, sizeof(response_buf),
            "[OK] PWM%u duty=%u%%", pwm_id, duty_on_percent);
    response_len = strlen((char *)response_buf);
}
```

---

## 对比分析

### 相同点

1. 都使用定时器的PWM功能控制风扇转速
2. 都支持通过命令手动控制风扇
3. 都使用占空比来调节风扇转速

### 不同点

| 特性 | Simu_STM32F407_UDP_FAN_Temp | 08 FAN_CTRL |
|------|---------------------------|-------------|
| 代码结构 | 模块化设计，单独的 fanctl.c 文件 | 代码直接写在 main.c 中 |
| 控制方式 | 支持温度自动控制和手动控制 | 仅支持手动控制 |
| 风扇分组 | 支持风扇分组管理，不同组使用不同温度曲线 | 无分组管理 |
| 温度曲线 | 支持分段温度曲线，根据温度自动调整转速 | 无温度曲线，需手动设置 |
| 调试模式 | 支持调试模式，方便手动测试 | 无专门的调试模式 |
| 错误检测 | 支持风扇锁定检测和温度异常检测 | 无错误检测机制 |

---

## 模块化改造建议

将 08 FAN_CTRL 工程的风扇控制代码模块化，创建 `fanctrl.c` 和 `fanctrl.h` 文件，参考 Simu_STM32F407_UDP_FAN_Temp 工程的设计：

1. **创建 fanctrl.h**：定义结构体和函数声明
2. **创建 fanctrl.c**：实现风扇控制逻辑
3. **修改 main.c**：使用模块化的风扇控制函数

### 改造后的优势

1. 代码结构更清晰，易于维护
2. 支持温度自动控制，提高系统可靠性
3. 便于扩展更多风扇控制功能
4. 减少 main.c 的代码量，提高可读性

---

## Simu_STM32F407_UDP_FAN_Temp 工程 - power_monitor.c 分析

### 文件概述

**power_monitor.c** 是电源监控模块，用于记录设备的开关机次数，实现开关机状态的延迟确认机制。

### 头文件包含（第1-7行）

```c
#include "power_monitor.h"    // 包含本模块的头文件，声明函数接口
#include <stdio.h>            // 标准输入输出库，用于printf等函数
#include "main.h"             // 主头文件，包含全局定义和配置
#include "logger.h"           // 日志模块，用于输出调试信息
#include "rt_param.h"         // 实时参数模块，存储开关机计数等参数
#include <stdbool.h>          // 布尔类型支持（true/false）
```

**语法说明**：
- `#include "xxx.h"`：包含项目自定义头文件
- `#include <xxx.h>`：包含系统标准库头文件
- 标准库用尖括号，自定义头文件用双引号

### 宏定义（第9行）

```c
#define POWER_MONITOR_SHUTDOWN_DELAY_SECONDS (30U)
```

**语法说明**：
- `#define`：预处理宏定义指令
- `(30U)`：U表示无符号整数（unsigned int）
- 宏定义在编译时进行文本替换

**功能说明**：
- 定义关机确认延迟时间为30秒
- 用于防止误关机或短暂断电导致的错误计数

### 静态变量定义（第11-12行）

```c
static bool s_pending = 0;           // 关机待确认标志
static uint16_t s_pending_seconds = 0;  // 关机倒计时计数器
```

**语法说明**：
- `static`：静态存储类别，限制变量作用域仅在当前文件
- `bool`：布尔类型，取值为true(1)或false(0)
- `uint16_t`：无符号16位整数类型（0-65535）

**功能说明**：
- `s_pending`：标记是否处于关机待确认状态
- `s_pending_seconds`：倒计时剩余秒数

### 函数声明（第14行）

```c
static void power_monitor_increment(int *counter);
```

**语法说明**：
- `static`：静态函数，只能在当前文件内调用
- `void`：函数无返回值
- `int *counter`：指向整数的指针参数

**功能说明**：
- 前置声明内部使用的计数器递增函数

### power_monitor_tick 函数（第16-33行）

```c
void power_monitor_tick(void)
{
    // 如果没有待确认的关机，直接返回
    if (s_pending == false)
    {
        return;
    }

    // 倒计时递减
    if (s_pending_seconds > 0U)
    {
        s_pending_seconds--;
    }

    // 倒计时结束，确认关机
    if (s_pending_seconds == 0U)
    {
        power_monitor_increment(&rt_param.power_off_count);  // 关机计数+1
        s_pending = false;                                   // 清除待确认标志
        save_rt_param_to_flash();                           // 保存到Flash
        log_info("Power monitor: normal shutdown recorded.");
    }
}
```

**语法说明**：
- `void`：函数无参数、无返回值
- `false`：布尔假值（0）
- `0U`：无符号整数0
- `&rt_param.power_off_count`：取地址运算符，获取变量的内存地址

**函数逻辑**：
1. 检查是否处于关机待确认状态
2. 如果是，递减倒计时计数器
3. 倒计时归零时：
   - 增加关机计数
   - 清除待确认标志
   - 保存参数到Flash
   - 记录日志

**调用时机**：
- 在 `main.c` 的 `loop_task_1000ms()` 函数中每秒调用一次

### power_monitor_on_power_on 函数（第35-46行）

```c
void power_monitor_on_power_on(void)
{
    // 如果有待确认的关机，取消它
    if (s_pending)
    {
        s_pending = 0;
        s_pending_seconds = 0;
        log_info("Power monitor: pending shutdown cancelled by power on.");
    }

    // 增加开机计数
    power_monitor_increment(&rt_param.power_on_count);
    save_rt_param_to_flash();
    log_info("Power monitor: power on count = %lu.", 
             (unsigned long)rt_param.power_on_count);
}
```

**功能说明**：
- 在设备开机时调用
- 如果有待确认的关机，取消它（防止误计数）
- 增加开机计数并保存

### power_monitor_on_power_off 函数（第48-54行）

```c
void power_monitor_on_power_off(void)
{
    s_pending = 1;                                          // 设置待确认标志
    s_pending_seconds = POWER_MONITOR_SHUTDOWN_DELAY_SECONDS;  // 设置30秒倒计时
    log_info("Power monitor: shutdown pending, will confirm in %u seconds.", 
             (unsigned int)POWER_MONITOR_SHUTDOWN_DELAY_SECONDS);
}
```

**功能说明**：
- 在接收到关机命令时调用
- 设置30秒倒计时，等待确认
- 如果在30秒内重新开机，则取消关机计数

### power_monitor_reset_counters 函数（第56-68行）

```c
void power_monitor_reset_counters(void)
{
    s_pending = 0;
    s_pending_seconds = 0;

    rt_param.power_on_count = 0;   // 清零开机计数
    rt_param.power_off_count = 0;  // 清零关机计数

    save_rt_param_to_flash();
    log_info("Power monitor: counters reset.");
}
```

**功能说明**：
- 重置开关机计数器
- 取消待确认关机状态
- 保存到Flash

### 计数器获取函数（第70-80行）

```c
int power_monitor_get_power_on_count(void)
{
    return rt_param.power_on_count;
}

int power_monitor_get_power_off_count(void)
{
    return rt_param.power_off_count;
}
```

**功能说明**：
- 提供接口获取开机/关机计数
- 用于外部查询或显示

### power_monitor_increment 函数（第83-95行）

```c
static void power_monitor_increment(int *counter)
{
    // 参数检查，防止空指针
    if (counter == NULL)
    {
        return;
    }

    // 防止溢出，最大值0xFFFFFFFF
    if (*counter != 0xFFFFFFFF)
    {
        (*counter)++;  // 递增计数器
    }
}
```

**语法说明**：
- `NULL`：空指针常量
- `*counter`：解引用指针，访问指针指向的值
- `(*counter)++`：先解引用，再递增
- `0xFFFFFFFF`：32位无符号整数的最大值

**功能说明**：
- 安全的计数器递增函数
- 包含空指针检查和溢出保护

---

## Simu_STM32F407_UDP_FAN_Temp 工程 - main.c 分析

### 文件概述

**main.c** 是系统主程序文件，包含：
- 全局变量定义
- 系统初始化
- 主循环任务调度
- 各周期任务的实现

### 头文件包含（第27-75行）

```c
#include "main.h"              // 主头文件，包含HAL库和全局定义
#include "adc.h"               // ADC驱动
#include "dma.h"               // DMA驱动
#include "i2c.h"               // I2C驱动
#include "tim.h"               // 定时器驱动
#include "usart.h"             // 串口驱动
#include "gpio.h"              // GPIO驱动
#include "spi.h"               // SPI驱动
#include "stm32f4xx_hal.h"     // HAL库核心头文件
```

**用户自定义头文件**：
```c
#include "lwip_comm.h"         // LWIP网络通信
#include "lan8720.h"           // 以太网PHY芯片驱动
#include "stmflash.h"          // Flash操作
#include "udp_demo.h"          // UDP演示
#include "fanctl.h"            // 风扇控制模块
#include "power_monitor.h"     // 电源监控模块
#include "modectl.h"           // 模式控制模块
#include "healthMgr.h"         // 健康管理模块
// ... 其他模块
```

**条件编译包含**：
```c
#if (projector_Model == LP92SLB || Projector_Model == NP72BN)
#include "it6635_import.h"     // IT6635 HDMI芯片驱动（特定型号）
#endif
```

**语法说明**：
- `#if`：条件编译，根据宏定义决定是否包含代码
- `||`：逻辑或运算符

### 全局变量定义（第80-130行）

```c
// 网络信息数组
uint8_t Ethernet_Info[20];

// 状态字符串变量
char Work_Status[3] = {"0"};       // 工作状态
char Temp_B_Status[8] = {"0"};     // 蓝色激光器温度
char Temp_G1_Status[8] = {"0"};    // 绿色激光器1温度
// ... 其他温度变量

// 系统状态标志
_Bool OE_lock;                     // 移轴锁定标志
VIDEO_INPUT_SRC_ Source_Stat;      // 视频源状态
_Bool video_src_changed = TRUE;    // 视频源改变标志

// 计数器
uint32_t secondCounter = 0;        // 秒计数器
```

**语法说明**：
- `_Bool`：C99标准布尔类型
- `TRUE/FALSE`：布尔值宏定义
- `uint32_t`：32位无符号整数

### 宏定义（第145-150行）

```c
#define SYSTEMTICK_PERIOD_MS 10    // 系统节拍周期10ms
#define FLASH_LATENCY_1 ((uint8_t)0x0001)  // Flash延迟1个时钟周期
```

### 外部变量声明（第158-168行）

```c
extern struct ip_addr rmtipaddr;           // 远程IP地址
extern uint8_t FAN_Temp_Error[7];          // 风扇温度错误数组
extern struct netif lwip_netif;            // LWIP网络接口
extern uint8_t EthInitStatus;              // 以太网初始化状态
```

**语法说明**：
- `extern`：声明外部定义的变量，告诉编译器变量在其他文件中定义

### 固件信息打印函数（第180-200行）

```c
const char* ProjectorModelName[] = {
    "NoneModel",
    "LP72BSL;NP52SLC;NP52SL;",
    "LP92SLB",
};

void print_fw_info(void)
{
    printf("*********************************************************************\n");
    printf("* Application for projector main Control on stm32f407zet6           *\n");
    printf("* Version: %d.%d.%d                                                    *\n",
           VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
    printf("* Compiled on %s at %s                               *\n", __DATE__, __TIME__);
    printf("*********************************************************************\n");

    printf("Projector Model: %s\n", ProjectorModelName[Projector_Model]);
}
```

**语法说明**：
- `const char*`：指向常量字符的指针（字符串）
- `__DATE__`：编译日期宏（格式：Mmm dd yyyy）
- `__TIME__`：编译时间宏（格式：hh:mm:ss）
- `%d`：printf格式符，输出十进制整数

### 100ms周期任务（第300-355行）

```c
void loop_task_100ms()
{
    /* 1. 移轴马达控制 100ms 检测周期*/
    if (!OE_lock && motor_nsleep && Work_Stat)
    {
        // 计算移轴位置
        OE_Shift_calculate();
        OE_Shift_AVE();
        
        // 移轴复位处理
        if (OE_Shift_Type == OE_Shift_RST)
        {
            OE_Shift_Reset();
        }
        else
        {
            // 移轴限位和移动处理
            OE_Shift_limit();
            OE_Shift_Handler();
            
            // 1秒无操作自动停止
            if (OE_timer >= OE_timer_cnt && OE_Shift_Type)
            {
                // 停止马达并清理状态
            }
        }
    }
}
```

**功能说明**：
- 每100ms执行一次
- 处理镜头移轴马达控制
- 包括位置计算、限位保护、自动停止

### 500ms周期任务（第357-370行）

```c
void loop_task_500ms()
{
    /* 1. 心跳灯 500ms 周期*/
    HEARTBEAT_BLINK();

    /* 2. 网络插入状态检测 500ms 周期*/
    if (LINK_UP != 0)
        Eth_Link_ITHandler(LAN8720_PHY_ADDRESS);

    /* 3. 异步方式，数据保存flash */
    do_save_rt_param_to_flash();
}
```

**功能说明**：
- 每500ms执行一次
- LED心跳灯闪烁（指示系统运行正常）
- 检测网线插拔状态
- 异步保存参数到Flash

### 1000ms周期任务 - 核心（第372-560行）

```c
void loop_task_1000ms()
{
    /* 1. Mstar 开机后，仅执行一次的任务 */
    if(mstar_work_on_oneshot_flag == 1)
    {
        mstar_work_on_oneshot_flag = 0;
        
        // 1.1: 同步信息到Mstar
        mstar_workon_info_sync_oneshot();

        // 1.2: mstar 开机后，配置3D延时芯片
        #if (Projector_Model == LP72BSL_NP52SLC_NP52SL)
        if(is_3d_delay_supported())
            init_delay3D();
        #endif
    }
```

**功能说明**：
- 每秒执行一次
- 处理开机后一次性任务

#### 风扇和温度控制（第500-520行）

```c
    /* 2. 温度检测， 风扇PWM调整，温度异常检测，风扇异常检测 （5s 周期） */
    if ((secondCounter % 5) == 0)  // 每5秒执行一次
    {
        if (Work_Stat)  // 仅在开机工作状态执行
        {
            int fan_err = 0;
            int temp_err = 0;
            
            // 检测风扇是否被锁定（堵转）
            for (size_t i = 1; i <= 8; i++)
            {
                Fan_LOCKED_check(i);
            }
            fan_err = Fan_Error_warn();
            
            // 获取温度传感器数据
            Get_Temperature();
            
            // 检测温度是否异常
            temp_err = Tempture_Erorr_Test();
            
            // 根据温度调整风扇PWM（核心调用）
            fan_pwm_auto_set(ADS_Temp_Data, sizeof(ADS_Temp_Data) / sizeof(float));
            
            // 如果有错误，通过PJLINK上报
            if (fan_err || temp_err)
            {
                PJLink_StatusNotify_ErrorStatus(fan_err, temp_err, 0, 0, 0, 0);
            }
        }
    }
```

**语法说明**：
- `secondCounter % 5`：取模运算，每5秒触发一次
- `sizeof(ADS_Temp_Data) / sizeof(float)`：计算数组元素个数
- `size_t`：无符号整数类型，用于数组索引

**函数调用关系**：
1. `Fan_LOCKED_check(i)`：检查第i个风扇是否堵转
2. `Fan_Error_warn()`：获取风扇错误状态
3. `Get_Temperature()`：读取所有温度传感器
4. `Tempture_Erorr_Test()`：检测温度是否超限
5. `fan_pwm_auto_set()`：**核心函数**，根据温度自动调整风扇转速
6. `PJLink_StatusNotify_ErrorStatus()`：上报错误状态

#### 健康状态上报（第522-540行）

```c
    /* 3. 周期XML上报 */
    if ((secondCounter % rt_param.health_report_period) == 0)
    {
        if (LINK_UP == 0x10 && rt_param.health_report_en)
        {
            if(Work_Stat == 0 && rt_param.xml_standby_report_en == 0)
            {
                ; // 待机状态下不进行XML上报
            }
            else{
                if (rt_param.health_report_format)
                    StateReportTask(STATE_REPORT_MODE_ACTIVE, STATE_REPORT_TYPE_XML);
                else
                    StateReportTask(STATE_REPORT_MODE_ACTIVE, STATE_REPORT_TYPE_BYTE);
            }
        }
    }
```

**功能说明**：
- 根据配置周期上报设备健康状态
- 支持XML格式和二进制格式

#### 电源监控调用（第558-560行）

```c
    /* 5. 秒计数器递增 */
    power_monitor_tick();  // 调用电源监控模块的tick函数
    secondCounter++;
```

**功能说明**：
- 每秒调用 `power_monitor_tick()`
- 处理关机延迟确认逻辑

### main 主函数（第567-896行）

#### 初始化阶段（第569-750行）

```c
int main(void)
{
    /* 1. Flash中断向量表重定位+使能全局中断 */
    RESET_IRQ_VEC_TAB_BASE(); 
    __enable_irq(); 

    /* 2. HAL库初始化 */
    HAL_Init();

    /* 3. 配置系统时钟 */
    SystemClock_Config();

    /* 4. 初始化GPIO */
    MX_GPIO_Init();

    /* 5. 优先配置uart，用于打印系统信息 */
    MX_USART1_UART_Init();
    MX_USART3_UART_Init();
```

**条件编译初始化（第620-650行）**：
```c
#if (Projector_Model == LP72BSL_NP52SLC_NP52SL)
    read_hw_version();  // 特定型号读取硬件版本
#endif

    print_fw_info();    // 打印固件信息
```

**Flash配置读取（第655-680行）**：
```c
    // 读取IAP更新参数
    STMFLASH_Read(FLASH_IAP_PARAM_ADDR, (uint32_t *)(&iapParam), 
                  sizeof(iapParam) / 4);
    
    // 读取网络配置
    STMFLASH_Read(ADDR_FLASH_SECTOR_6, (uint32_t *)Ethernet_Info, 9);
    
    // 读取其他配置
    read_rt_param_from_flash();
```

**外设初始化（第690-730行）**：
```c
    MX_DMA_Init();
    MX_ADC1_Init();
    MX_I2C1_Init();
    MX_I2C2_Init();
    MX_TIM1_Init();    // 风扇PWM定时器1
    MX_TIM4_Init();    // 风扇PWM定时器4
    MX_TIM3_Init();    // 风扇PWM定时器3
    MX_TIM2_Init();    // 风扇PWM定时器2
    MX_TIM6_Init();    // 系统节拍定时器

    // 启动外设
    Timer_Base_enable();
    HAL_ADC_Start_DMA(&hadc1, (uint32_t *)ADC_ConvertedValue, ADC_NUMOFCHANNEL);
    HAL_TIM_Base_Start_IT(&htim6);
    Timer_Pwm_enable();
```

**工作模式初始化（第732-734行）**：
```c
    // 外设初始化完成，初始化工作模式，风扇开始运转
    modectl_init_mode(rt_param.work_mode);
```

**看门狗初始化（第750-760行）**：
```c
    // 设置iwatch dog 定时器, 超时周期 8s
    iwdg.Instance = IWDG;
    iwdg.Init.Prescaler = IWDG_PRESCALER_128;
    iwdg.Init.Reload = 4095;
    HAL_IWDG_Init(&iwdg);
```

#### 主循环（第770-896行）

```c
    log_info("Enter main loop");
    while (1)
    {
        /* 100ms 周期任务处理段 */
        if (loopFlag_100ms)
        {
            loopFlag_100ms = 0;
            loop_task_100ms();
        }
       
        /* 500ms 周期任务处理段 */
        if (loopFlag_500ms)
        {
            loopFlag_500ms = 0;
            loop_task_500ms();
        }

        /* 1000ms 周期任务处理段 */
        if (loopFlag_1000ms)
        {
            loopFlag_1000ms = 0;
            loop_task_1000ms();
            HAL_IWDG_Refresh(&iwdg);  // 喂狗
        }

        // lwip周期任务
        lwip_periodic_handle();
        
        // I2C指令包处理
        I2C_CMD_Process();
        
        // 网络指令包处理
        Netif_CMD_Process();
        
        // UART命令处理
        uartCmdProcTask();
    }
}
```

**系统架构说明**：
- 采用 **Round-robin（轮询）+ ISR（中断服务）** 架构
- 定时器中断设置标志位（loopFlag_xxx）
- 主循环查询标志位，执行对应任务
- 避免使用RTOS，减少系统开销

---

## Projector_Model 和 Non_Model 详解

### 定义位置

在 [pj_hw_def.h](file:///e:/YwcFiles/项目文件/EdgeMCU_Keil/52vsEdge/Simu_STM32F407_UDP_FAN_Temp/User/src/funs/inc/pj_hw_def.h) 文件中定义：

```c
// 投影仪型号枚举值
#define Non_Model                 0x0 
#define LP72BSL_NP52SLC_NP52SL    0x1       
#define LP92SLB                   0x2       
#define NP52SL                    0x3       
#define NP72BN                    0x4       

// 当前选择的型号（编译时确定）
#define Projector_Model LP72BSL_NP52SLC_NP52SL
```

### 类型说明

- **Projector_Model**：宏定义，值为当前选择的型号
- **Non_Model**：宏定义，值为0x0，表示"无型号"或"通用型号"
- **类型**：两者都是整数常量宏（int类型）

### 用途

1. **条件编译**：根据型号包含/排除特定代码
2. **硬件适配**：不同型号使用不同的硬件配置
3. **功能裁剪**：某些功能只在特定型号上支持

### 当前配置分析

当前配置：
```c
#define Projector_Model LP72BSL_NP52SLC_NP52SL  // 值为0x1
```

在 `fan_pwm_auto_set` 函数中：
```c
#if(Projector_Model == Non_Model)  // 0x1 == 0x0，条件为假
    // 自动温度控制代码（不会编译）
#else
    // 固定转速代码（会编译执行）
#endif
```

**结论**：当前代码执行的是 `#else` 分支的**固定转速模式**，而非自动温度控制模式。

---

## 函数调用关系图

```
main()
├── HAL_Init()                          // HAL库初始化
├── SystemClock_Config()                // 系统时钟配置
├── MX_GPIO_Init()                      // GPIO初始化
├── MX_USART1_UART_Init()               // 串口1初始化
├── MX_TIMx_Init()                      // 定时器初始化
├── modectl_init_mode()                 // 模式初始化（启动风扇）
│   └── FAN_PWM_Set()                   // 设置初始PWM
│
└── while(1) 主循环
    ├── loop_task_100ms()               // 100ms任务
    │   └── OE_Shift_Handler()          // 移轴控制
    │
    ├── loop_task_500ms()               // 500ms任务
    │   └── do_save_rt_param_to_flash() // 参数保存
    │
    ├── loop_task_1000ms()              // 1s任务
    │   ├── mstar_workon_info_sync_oneshot()  // 开机同步
    │   ├── Get_Temperature()           // 获取温度
    │   ├── Fan_LOCKED_check()          // 风扇堵转检测
    │   ├── fan_pwm_auto_set()          // 风扇自动控制
    │   │   ├── calc_fan_pwm()          // 计算PWM值
    │   │   └── __HAL_TIM_SET_COMPARE() // 设置PWM
    │   ├── power_monitor_tick()        // 电源监控
    │   └── StateReportTask()           // 状态上报
    │
    ├── I2C_CMD_Process()               // I2C命令处理
    ├── Netif_CMD_Process()             // 网络命令处理
    └── uartCmdProcTask()               // UART命令处理
```

---

## 总结

### power_monitor.c 核心要点

1. **延迟确认机制**：关机后等待30秒确认，防止误计数
2. **数据持久化**：计数器保存到Flash，断电不丢失
3. **安全设计**：包含空指针检查和溢出保护

### main.c 核心要点

1. **分层架构**：HAL层 → 驱动层 → 应用层
2. **时间片轮询**：100ms/500ms/1s 三级任务调度
3. **风扇控制集成**：每5秒调用 `fan_pwm_auto_set()` 进行温度控制
4. **条件编译**：通过 `Projector_Model` 适配不同硬件

### 当前执行路径

由于 `Projector_Model == LP72BSL_NP52SLC_NP52SL` (0x1)，不等于 `Non_Model` (0x0)：
- `fan_pwm_auto_set()` 中的自动温度控制代码**不会执行**
- 执行的是固定转速模式（70%或100%）
- 如需启用自动温度控制，需要修改 `pj_hw_def.h` 中的型号定义

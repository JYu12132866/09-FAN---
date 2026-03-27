# 仿真机JSON-RPC接口规范

## 文档说明

本文档详细描述了STM32F407投影仿真机的JSON-RPC接口规范，包含设备信息、光源控制、显示设置、网络配置等全部配置项目。

## 接口分级说明

- **一级配置（Level 1）**：基础设备信息
- **二级配置（Level 2）**：工厂级光源参数
- **三级配置（Level 3）**：高级显示和网络配置
- **四级配置（Level 4）**：用户级显示和颜色配置

---

## 一级配置（Level 1）

### 1. 型号

**功能**：设置/获取设备型号和显示模式

#### 设置型号
```json
// 请求
{
  "jsonrpc": "2.0",
  "method": "info.model",
  "params": {
    "mode": "set",
    "display_mode": "lp50sl"
  },
  "id": 3
}

// 响应
{
  "jsonrpc": "2.0",
  "result": {
    "return": true
  },
  "id": 3
}
```

#### 获取型号
```json
// 请求
{
  "jsonrpc": "2.0",
  "method": "info.model",
  "params": {
    "mode": "get"
  },
  "id": 3
}

// 响应
{
  "jsonrpc": "2.0",
  "result": {
    "model": "np52sl",
    "display_mode": "lp50sl"
  },
  "id": 3
}
```

**备注**：用于内部识别和显示

---

### 2. 序列号

**功能**：获取设备序列号

```json
// 请求
{
  "jsonrpc": "2.0",
  "method": "info.serial",
  "params": {
    "mode": "get"
  },
  "id": 3
}

// 响应
{
  "jsonrpc": "2.0",
  "result": {
    "serial": "20241234567"
  },
  "id": 3
}
```

---

### 3. 生产日期

**功能**：设置/获取设备生产日期

#### 设置日期
```json
// 请求
{
  "jsonrpc": "2.0",
  "method": "info.date",
  "params": {
    "mode": "set",
    "date": "20250101"
  },
  "id": 3
}

// 响应
{
  "jsonrpc": "2.0",
  "result": {
    "return": true
  },
  "id": 3
}
```

#### 获取日期
```json
// 请求
{
  "jsonrpc": "2.0",
  "method": "info.date",
  "params": {
    "mode": "get"
  },
  "id": 3
}

// 响应
{
  "jsonrpc": "2.0",
  "result": {
    "date": "20250101"
  },
  "id": 3
}
```

---

### 4. 硬件版本

**功能**：获取硬件版本信息

```json
// 请求
{
  "jsonrpc": "2.0",
  "method": "info.hw_version",
  "params": {
    "mode": "get"
  },
  "id": 3
}

// 响应
{
  "jsonrpc": "2.0",
  "result": {
    "hw_mainboard": "v1.1.1"
  },
  "id": 3
}
```

---

### 5. 软件版本

**功能**：获取固件版本信息

```json
// 请求
{
  "jsonrpc": "2.0",
  "method": "info.version",
  "params": {
    "mode": "get"
  },
  "id": 3
}

// 响应
{
  "jsonrpc": "2.0",
  "result": {
    "fw_mcu": "v1.1.1",
    "fw_mstar": "v1.2.1",
    "fw_dlp": "v1.2.1"
  },
  "id": 3
}
```

---

### 6. 使用时间

**功能**：获取设备累计工作时间

```json
// 请求
{
  "jsonrpc": "2.0",
  "method": "fty.worktime",
  "params": {
    "mode": "get"
  },
  "id": 4
}

// 响应
{
  "jsonrpc": "2.0",
  "result": {
    "time": "12.34"
  },
  "id": 4
}
```

**单位**：小时

---

### 7. 异常关机

**功能**：清空/获取异常关机次数

#### 清空计数
```json
// 请求
{
  "jsonrpc": "2.0",
  "method": "info.error_poweroff",
  "params": {
    "mode": "set"
  },
  "id": 3
}

// 响应
{
  "jsonrpc": "2.0",
  "result": {
    "return": true
  },
  "id": 3
}
```

#### 获取计数
```json
// 请求
{
  "jsonrpc": "2.0",
  "method": "info.error_poweroff",
  "params": {
    "mode": "get"
  },
  "id": 3
}

// 响应
{
  "jsonrpc": "2.0",
  "result": {
    "error_poweroff": "10"
  },
  "id": 3
}
```

---

## 二级配置（Level 2）

### 8. 光源参数

**功能**：设置/获取RGB光源PWM亮度

#### 设置光源亮度
```json
// 设置红光
{
  "jsonrpc": "2.0",
  "method": "fty.laser.bright",
  "params": {
    "mode": "set",
    "source": "r",
    "value": 1000
  },
  "id": 4
}

// 设置绿光
{
  "jsonrpc": "2.0",
  "method": "fty.laser.bright",
  "params": {
    "mode": "set",
    "source": "g",
    "value": 1000
  },
  "id": 4
}

// 设置蓝光
{
  "jsonrpc": "2.0",
  "method": "fty.laser.bright",
  "params": {
    "mode": "set",
    "source": "b",
    "value": 1000
  },
  "id": 4
}

// 响应
{
  "jsonrpc": "2.0",
  "result": {
    "return": true
  },
  "id": 4
}
```

#### 获取光源亮度
```json
// 请求
{
  "jsonrpc": "2.0",
  "method": "fty.laser.bright",
  "params": {
    "mode": "get"
  },
  "id": 4
}

// 响应
{
  "jsonrpc": "2.0",
  "result": {
    "r": 1000,
    "g": 1000,
    "b": 1000
  },
  "id": 4
}
```

**参数范围**：0-1000

---

### 9. 亮度等级数据

**功能**：传输100阶亮度参数数据（透传到Mstar）

#### 设置亮度数据
```json
// 请求
{
  "jsonrpc": "2.0",
  "method": "route.mstar",
  "params": {
    "value": "SGVsbG8sIEJhc2U2NCE="
  },
  "id": 4
}

// 响应
{
  "jsonrpc": "2.0",
  "result": {
    "return": true
  },
  "id": 4
}
```

#### 获取亮度数据（通知）
```json
// Mstar主动通知
{
  "jsonrpc": "2.0",
  "method": "mstar.notify",
  "params": {
    "type": "transparent",
    "data": "SGVsbG8gV29ybGQ=",
    "timestamp": 1234567890
  }
}
```

**备注**：数据使用Base64编码传输

---

## 三级配置（Level 3）

### 10. HSG数据

**功能**：设置/获取HSG（色相/饱和度/增益）参数

#### 设置HSG参数
```json
// 设置红色HSG
{
  "jsonrpc": "2.0",
  "method": "fty.hsg",
  "params": {
    "mode": "set",
    "color": "r",
    "attr": "hue",
    "value": 100
  },
  "id": 4
}

// 设置绿色饱和度
{
  "jsonrpc": "2.0",
  "method": "fty.hsg",
  "params": {
    "mode": "set",
    "color": "g",
    "attr": "satu",
    "value": 100
  },
  "id": 4
}

// 设置蓝色增益
{
  "jsonrpc": "2.0",
  "method": "fty.hsg",
  "params": {
    "mode": "set",
    "color": "b",
    "attr": "gain",
    "value": 100
  },
  "id": 4
}

// 设置白色R分量
{
  "jsonrpc": "2.0",
  "method": "fty.hsg",
  "params": {
    "mode": "set",
    "color": "w",
    "attr": "r",
    "value": 100
  },
  "id": 4
}

// 响应
{
  "jsonrpc": "2.0",
  "result": {
    "return": true
  },
  "id": 4
}
```

#### 获取HSG参数
```json
// 请求
{
  "jsonrpc": "2.0",
  "method": "fty.hsg",
  "params": {
    "mode": "get"
  },
  "id": 4
}

// 响应
{
  "jsonrpc": "2.0",
  "result": {
    "r": {"h": 100, "s": 100, "g": 100},
    "g": {"h": 100, "s": 100, "g": 100},
    "b": {"h": 100, "s": 100, "g": 100},
    "c": {"h": 100, "s": 100, "g": 100},
    "y": {"h": 100, "s": 100, "g": 100},
    "m": {"h": 100, "s": 100, "g": 100},
    "w": {"r": 100, "g": 100, "b": 100}
  },
  "id": 4
}
```

**支持颜色**：r（红）、g（绿）、b（蓝）、c（青）、y（黄）、m（品红）、w（白）

---

### 11-18. 用户三级设置

**功能**：批量设置/获取三级用户配置

#### 设置三级配置
```json
// 请求
{
  "jsonrpc": "2.0",
  "method": "user.set_L3",
  "params": {
    "mode": "set",
    "color_mode": "BT709",
    "color_temp": "9300K",
    "brightness_level": 80,
    "display_mode": "2D",
    "LD_light": true,
    "IR_light": false,
    "installation_mode": "front",
    "high_altitude": true
  },
  "id": 4
}

// 响应
{
  "jsonrpc": "2.0",
  "result": {
    "return": true
  },
  "id": 4
}
```

#### 获取三级配置
```json
// 请求
{
  "jsonrpc": "2.0",
  "method": "user.set_L3",
  "params": {
    "mode": "get"
  },
  "id": 4
}

// 响应
{
  "jsonrpc": "2.0",
  "result": {
    "color_mode": "BT709",
    "color_temp": "9300K",
    "brightness_level": 80,
    "display_mode": "2D",
    "LD_light": true,
    "IR_light": false,
    "installation_mode": "front",
    "high_altitude": true
  },
  "id": 4
}
```

#### 参数说明

| 参数 | 说明 | 可选值 |
|------|------|--------|
| color_mode | 颜色模式 | BT709, P3, BT2020, HighBright |
| color_temp | 色温 | 9300K, 7500K, 6500K, HSG |
| brightness_level | 亮度等级 | 0-100 |
| display_mode | 显示模式 | 2D, 3D_in, 3D_ext, Smear, 3D stereo, Low_latency |
| LD_light | 可见光状态 | true, false |
| IR_light | 红外光状态 | true, false |
| installation_mode | 安装模式 | front, rear, ceiling_front, ceiling_rear |
| high_altitude | 高海拔模式 | true, false |

---

### 19-23. 网络配置

**功能**：设置/获取网络参数

#### 设置网络配置
```json
// 请求
{
  "jsonrpc": "2.0",
  "method": "com.ip",
  "params": {
    "mode": "set",
    "ipaddress": "192.168.0.20",
    "ipmask": "255.255.255.0",
    "ipgateway": "192.168.0.1",
    "reip": "192.168.0.100",
    "udpport": "8080"
  },
  "id": 4
}

// 响应
{
  "jsonrpc": "2.0",
  "result": {
    "return": true
  },
  "id": 4
}
```

#### 获取网络配置
```json
// 请求
{
  "jsonrpc": "2.0",
  "method": "com.ip",
  "params": {
    "mode": "get"
  },
  "id": 4
}

// 响应
{
  "jsonrpc": "2.0",
  "result": {
    "ipaddress": "192.168.0.20",
    "ipmask": "255.255.255.0",
    "ipgateway": "192.168.0.1",
    "reip": "192.168.0.100",
    "udpport": "8080"
  },
  "id": 4
}
```

#### 参数说明

| 参数 | 说明 |
|------|------|
| ipaddress | 本机IP地址 |
| ipmask | 子网掩码 |
| ipgateway | 网关地址 |
| reip | 目标IP地址 |
| udpport | UDP端口号 |

---

### 25-27. 信息上报配置

**功能**：设置/获取信息上报参数

#### 设置上报配置
```json
// 请求
{
  "jsonrpc": "2.0",
  "method": "com.packet",
  "params": {
    "mode": "set",
    "packet_enable": true,
    "packet_type": "xml",
    "packet_interval": 30
  },
  "id": 4
}

// 响应
{
  "jsonrpc": "2.0",
  "result": {
    "return": true
  },
  "id": 4
}
```

#### 获取上报配置
```json
// 请求
{
  "jsonrpc": "2.0",
  "method": "com.packet",
  "params": {
    "mode": "get"
  },
  "id": 4
}

// 响应
{
  "jsonrpc": "2.0",
  "result": {
    "packet_enable": true,
    "packet_type": "xml",
    "packet_interval": 30
  },
  "id": 4
}
```

#### 参数说明

| 参数 | 说明 | 可选值 |
|------|------|--------|
| packet_enable | 上报开关 | true, false |
| packet_type | 报文格式 | xml, byte |
| packet_interval | 上报间隔（秒） | 15, 30, 45 |

---

## 四级配置（Level 4）

### 28-43. 用户四级设置

**功能**：批量设置/获取四级用户配置

#### 设置四级配置
```json
// 请求
{
  "jsonrpc": "2.0",
  "method": "user.set_L4",
  "params": {
    "mode": "set",
    "source": "VGA",
    "input_color_format": "RGB",
    "color_domain": "RGB",
    "color_ctrs": 76,
    "color_sharp": 70,
    "color_bright": 50,
    "color_gamma": "2.2",
    "background": "blue",
    "ir_id": 0,
    "lens_lock": false,
    "warn_exec": true,
    "warn_osd": true,
    "no_input_shutdown": true,
    "aspect_ratio": "full",
    "language": "chinese",
    "power_on": true
  },
  "id": 4
}

// 响应
{
  "jsonrpc": "2.0",
  "result": {
    "return": true
  },
  "id": 4
}
```

#### 获取四级配置
```json
// 请求
{
  "jsonrpc": "2.0",
  "method": "user.set_L4",
  "params": {
    "mode": "get"
  },
  "id": 4
}

// 响应
{
  "jsonrpc": "2.0",
  "result": {
    "source": "VGA",
    "input_color_format": "RGB",
    "color_domain": "RGB",
    "color_ctrs": 76,
    "color_sharp": 70,
    "color_bright": 50,
    "color_gamma": "2.2",
    "background": "blue",
    "ir_id": 0,
    "lens_lock": false,
    "warn_exec": true,
    "warn_osd": true,
    "no_input_shutdown": true,
    "aspect_ratio": "full",
    "language": "chinese",
    "power_on": true
  },
  "id": 4
}
```

#### 参数说明

| 参数 | 说明 | 可选值 |
|------|------|--------|
| source | 输入源 | auto, VGA, DP, HDMI1, DVI |
| input_color_format | 输入颜色格式 | RGB, YPbPr, Auto |
| color_domain | 色域 | RGB, YUV |
| color_ctrs | 对比度 | 0-100 |
| color_sharp | 清晰度 | 0-100 |
| color_bright | 背景亮度 | 0-100 |
| color_gamma | 伽玛值 | 1.8, 2.0, 2.2, 2.4, off |
| background | 背景色 | red, green, blue, white, black |
| ir_id | 红外ID | 0-10 |
| lens_lock | 镜头锁定 | true, false |
| warn_exec | 报警检测 | true, false |
| warn_osd | 告警提示 | true, false |
| no_input_shutdown | 无输入关机 | true, false |
| aspect_ratio | 宽高比 | full, auto, 4:3, 16:10, 16:9, 1:1 |
| language | 语言 | chinese, english |
| power_on | 上电开机 | true, false |

---

### 44-45. 颜色四级设置

**功能**：设置/获取颜色增益和独立颜色调节

#### 设置颜色配置
```json
// 请求
{
  "jsonrpc": "2.0",
  "method": "user.color_set_L4",
  "params": {
    "mode": "set",
    "gain_r": 100,
    "gain_g": 100,
    "gain_b": 100,
    "indep_color_r": 50,
    "indep_color_g": 50,
    "indep_color_b": 50,
    "indep_color_c": 50,
    "indep_color_m": 50,
    "indep_color_y": 50
  },
  "id": 4
}

// 响应
{
  "jsonrpc": "2.0",
  "result": {
    "return": true
  },
  "id": 4
}
```

#### 获取颜色配置
```json
// 请求
{
  "jsonrpc": "2.0",
  "method": "user.color_set_L4",
  "params": {
    "mode": "get"
  },
  "id": 4
}

// 响应
{
  "jsonrpc": "2.0",
  "result": {
    "gain_r": 100,
    "gain_g": 100,
    "gain_b": 100,
    "indep_color_r": 50,
    "indep_color_g": 50,
    "indep_color_b": 50,
    "indep_color_c": 50,
    "indep_color_m": 50,
    "indep_color_y": 50
  },
  "id": 4
}
```

#### 参数说明

| 参数 | 说明 | 范围 |
|------|------|------|
| gain_r | 红色增益 | 0-200 |
| gain_g | 绿色增益 | 0-200 |
| gain_b | 蓝色增益 | 0-200 |
| indep_color_r | 独立颜色-红 | 0-100 |
| indep_color_g | 独立颜色-绿 | 0-100 |
| indep_color_b | 独立颜色-蓝 | 0-100 |
| indep_color_c | 独立颜色-青 | 0-100 |
| indep_color_m | 独立颜色-品红 | 0-100 |
| indep_color_y | 独立颜色-黄 | 0-100 |

---

## API方法索引

### 信息类（info.*）
- `info.model` - 型号设置/获取
- `info.serial` - 序列号获取
- `info.date` - 生产日期设置/获取
- `info.version` - 软件版本获取
- `info.hw_version` - 硬件版本获取
- `info.error_poweroff` - 异常关机清空/获取

### 工厂类（fty.*）
- `fty.laser.bright` - 光源亮度调节
- `fty.hsg` - HSG参数调节
- `fty.worktime` - 使用时间获取

### 用户设置类（user.*）
- `user.set_L3` - 三级用户配置
- `user.set_L4` - 四级用户配置
- `user.color_set_L4` - 四级颜色配置

### 通信类（com.*）
- `com.ip` - 网络IP配置
- `com.packet` - 信息上报配置

### 路由类（route.*）
- `route.mstar` - Mstar透传数据

### 通知类（*.notify）
- `mstar.notify` - Mstar主动通知

---

## 版本历史

| 版本 | 日期 | 说明 |
|------|------|------|
| v1.0 | 2025-11 | 初始版本 |

---

## 附录

### 通用响应格式

成功响应：
```json
{
  "jsonrpc": "2.0",
  "result": {
    "return": true
  },
  "id": <request_id>
}
```

错误响应：
```json
{
  "jsonrpc": "2.0",
  "error": {
    "code": -32600,
    "message": "Invalid Request"
  },
  "id": <request_id>
}
```

### 常用错误码

| 错误码 | 说明 |
|--------|------|
| -32700 | 解析错误 |
| -32600 | 无效请求 |
| -32601 | 方法不存在 |
| -32602 | 无效参数 |
| -32603 | 内部错误 |

---

**文档维护**：请在修改接口时同步更新本文档


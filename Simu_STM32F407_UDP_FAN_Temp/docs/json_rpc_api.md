# JSON-RPC 2.0 接口协议说明

## 概述

本项目基于 JSON-RPC 2.0 协议，通过 UDP 链路为上位机提供投影机控制与状态查询能力。所有请求和响应均使用 UTF-8 编码的 JSON 字符串，采用无连接的 UDP 报文进行交互。除非特别说明，接口均为同步请求-响应模式，成功时返回 `result` 对象，失败时返回 `error` 对象。

典型请求报文：

```json
{
  "jsonrpc": "2.0",
  "method": "comm.power",
  "id": 1001,
  "params": {
    "mode": "get"
  }
}
```

成功响应报文：

```json
{
  "jsonrpc": "2.0",
  "result": {
    "state": true
  },
  "id": 1001
}
```

失败响应报文：

```json
{
  "jsonrpc": "2.0",
  "error": {
    "code": -32602,
    "message": "Invalid params"
  },
  "id": 1001
}
```

## 通用约定

- `jsonrpc` 固定为字符串 `"2.0"`。
- `id` 为整数，建议由上位机生成并递增，错误请求导致的解析失败时可能返回 `null`。
- `method` 为注册在固件内的 RPC 方法名，详见后续章节。
- `params` 必须是对象类型。若方法不需要参数，也需要发送空对象 `{}`。
- 所有布尔值均使用 JSON 标准的 `true`/`false`。
- 数值字段除非额外说明，均以十进制整型传输。
- 返回对象中的 `return` 字段表示底层指令发送结果，`true` 为执行成功，`false` 表示外设未响应或执行失败。

## 错误码

| 错误码 | 名称                    | 说明                                   | 可能触发场景 |
| ------ | ----------------------- | -------------------------------------- | ------------ |
| -32700 | `Parse error`           | JSON 解析失败                          | 非法 JSON 报文 |
| -32600 | `Invalid Request`       | 协议格式错误（字段缺失或类型不匹配）   | `jsonrpc` 非字符串、`params` 非对象等 |
| -32601 | `Method not found`      | 未注册的方法名                         | `method` 拼写错误 |
| -32602 | `Invalid params`        | 参数值非法或不支持的取值               | 传入未定义的模式、取值超范围 |
| -32603 | `Internal error`        | 固件内部错误                           | 预留，代码中暂未直接使用 |
| -32000 | `Server error`          | 下位机执行失败或外设超时               | UART 回包超时、底层执行失败 |

## 参数与返回字段规范

- 除个别特殊说明外，方法均依赖 `params.mode` 区分操作模式，如 `get`、`set`、`reset`。
- `params` 中未被使用的字段即便存在也不会生效，建议仅传递必要参数。
- 响应中的字段均以实际含义命名，数值单位在各方法描述中注明。

## 接口详情

以下接口按照在 `rpcFuncTab` 中的注册顺序列出。所有接口均可通过 UDP 调用，`method` 名称区分大小写。

### `comm.power`

- **用途**：开关机控制与状态查询。
- **模式与参数**：
  - `mode: "set"`：需提供 `state` (bool)，`true` 表示开机，`false` 表示关机。
  - `mode: "get"`：无需额外字段。
- **响应**：
  - `set`：`{ "return": true/false }`。
  - `get`：`{ "state": true/false }`。
- **异常**：
  - `state` 非布尔或 `mode` 非法 → `Invalid Request`。
  - UART 发送失败 → `Server error`。

### `video.background`

- **用途**：设置无信号背景色。
- **模式**：`mode: "set"`。
- **参数**：`color` 字符串，取值 `red` / `green` / `blue` / `write`（白色） / `black`。
- **响应**：`{ "return": true/false }`。
- **异常**：非法颜色或类型错误 → `Invalid params`。

### `lens.focus`

- **用途**：镜头聚焦电机控制。
- **模式**：`mode: "set"`。
- **参数**：
  - `dir`：`"futher"`（远焦）或 `"closer"`（近焦）。
  - `step`：数值型，固件用于参数校验（当前版本未直接使用）。
- **响应**：`{ "return": true/false }`。
- **异常**：缺少必填字段或方向非法 → `Invalid params`。

### `lens.zoom`

- **用途**：镜头变焦控制。
- **模式**：`mode: "set"`。
- **参数**：
  - `dir`：`"plus"`（放大）或 `"minus"`（缩小）。
  - `step`：数值型，当前仅做校验。
- **响应**：`{ "return": true/false }`。
- **异常**：取值非法 → `Invalid params`。

### `lens.shift`

- **用途**：镜头位移以及复位。
- **模式与参数**：
  - `mode: "set"`：`dir` 取值 `up`/`down`/`left`/`right`，`step` 数值型（校验用）。
  - `mode: "reset"`：无需其他字段。
- **响应**：`{ "return": true/false }`。
- **异常**：方向非法或模式不支持 → `Invalid params`。

### `lens.lock`

- **用途**：镜头锁定功能。
- **模式**：`mode: "set"`。
- **参数**：`lock` 布尔型，`true` 锁定、`false` 解锁。
- **响应**：`{ "return": true/false }`。
- **异常**：布尔类型错误 → `Invalid Request`。

### `color.bright`

- **用途**：亮度设置与查询。
- **模式与参数**：
  - `mode: "set"`：`value` 数值（0~255 建议）。
  - `mode: "get"`：无需额外字段。
- **响应**：
  - `set`：`{ "return": true/false }`。
  - `get`：`{ "value": <0-255> }`。
- **异常**：数值非法 → `Invalid params`; 底层超时 → `Server error`。

### `color.ctrs`

- 与 `color.bright` 类似，调节对象为对比度，字段同名。

### `color.sharp`

- 与 `color.bright` 类似，调节对象为清晰度，字段同名。

### `color.gamma`

- 与 `color.bright` 类似，调节对象为 Gamma，字段同名。

### `commu_mac`

- **用途**：查询当前网络 MAC 地址。
- **模式**：`mode: "get"`。
- **响应**：`{ "mac": "xx:xx:xx:xx:xx:xx" }`。
- **异常**：不支持 `set`，误传模式会得到 `Invalid params`。

### `misc.testpattern`

- **用途**：测试图输出控制。
- **模式**：`mode: "set"`。
- **参数**：`pattern` 取值 `exit` / `red` / `green` / `blue` / `black` / `write` / `gray` / `checkboard` / `whitegrid`。
- **响应**：`{ "return": true/false }`。
- **异常**：未知图案 → `Invalid params`。

### `misc.hialt`

- **用途**：高海拔模式控制。
- **模式与参数**：
  - `mode: "set"`：`state` 布尔型。
  - `mode: "get"`：返回当前状态。
- **响应**：
  - `set`：`{ "return": true/false }`。
  - `get`：`{ "state": true/false }`。
- **异常**：类型错误 → `Invalid Request`。

### `light.laser.switch`

- **用途**：可见光激光光源开关。
- **模式**：`mode: "set"`。
- **参数**：`state` 布尔型。
- **响应**：`{ "return": true/false }`。

### `light.ir.switch`

- **用途**：红外光源开关。
- **模式**：`mode: "set"`。
- **参数**：`state` 布尔型。
- **响应**：`{ "return": true/false }`。

### `light.ir.bright`

- **用途**：红外光源亮度调节。
- **模式与参数**：
  - `mode: "set"`：`value` 数值（建议 0~255）。
  - `mode: "get"`：返回当前亮度。
- **响应**：
  - `set`：`{ "return": true/false }`。
  - `get`：`{ "value": <0-255> }`。

### `info.model`

- **用途**：查询产品型号。
- **模式**：`mode: "get"`。
- **响应**：`{ "model": "<字符串>" }`。
- **异常**：底层读取失败 → `Server error`。

### `info.serial`

- **用途**：查询序列号。
- **模式**：`mode: "get"`。
- **响应**：`{ "serial": "<字符串>" }`。

### `info.version`

- **用途**：查询固件版本信息。
- **模式**：`mode: "get"`。
- **响应**：
  ```json
  {
    "fw_mcu": "Vx.y.z",
    "fw_mstar": "Va.b.c",
    "fw_dlp": "Vd.e.f"
  }
  ```
- **说明**：`fw_mcu` 由 MCU 固件本地版本号组成，其余来源于外设回传。

### `info.temp`

- **用途**：查询多路温度传感器读数。
- **模式**：`mode: "get"`。
- **响应**：
  ```json
  {
    "R1": <int>,
    "R2": <int>,
    "G1": <int>,
    "G2": <int>,
    "B1": <int>,
    "B2": <int>,
    "ENV": <int>,
    "DMD": <int>
  }
  ```
- **单位**：摄氏度（整数）。

### `fty.worktime`

- **用途**：查询累计运行时间。
- **模式**：`mode: "get"`。
- **响应**：`{ "worktime": <uint16> }`，单位为小时。

### `fty.hsg`

- **用途**：工厂色彩校正参数调整。
- **模式与参数**：
  - `mode: "set"`：`color`（`r/g/b/c/y/m/w`），`attr`（`hue`/`satu`/`gain` 或 `r/g/b`），`value` 数值（0~255 建议）。
  - `mode: "get"`：返回所有颜色分量。
  - `mode: "reset"`：恢复默认，需仅提供 `mode`。
- **响应**：
  - `set`/`reset`：`{ "return": true/false }`。
  - `get`：键为颜色，值为对象（示例）：
    ```json
    {
      "R": { "H": 0, "S": 0, "G": 0 },
      "G": { "H": 0, "S": 0, "G": 0 },
      "B": { "H": 0, "S": 0, "G": 0 },
      "C": { "H": 0, "S": 0, "G": 0 },
      "Y": { "H": 0, "S": 0, "G": 0 },
      "M": { "H": 0, "S": 0, "G": 0 },
      "W": { "R": 0, "G": 0, "B": 0 }
    }
    ```
- **异常**：参数组合非法 → `Invalid params`。

### `fty.laser.bright`

- **用途**：工厂激光光源亮度校准。
- **模式与参数**：
  - `mode: "set"`：`source`（`r/g/b`），`value` 数值（0~65535 建议）。
  - `mode: "get"`：返回 `R/G/B` 三个分量。
  - `mode: "reset"`：恢复默认。
- **响应**：
  - `set`/`reset`：`{ "return": true/false }`。
  - `get`：`{ "R": <uint16>, "G": <uint16>, "B": <uint16> }`。

### `fty.hwcfg`

- **用途**：硬件配置写入（当前仅支持镜头电机类型）。
- **模式**：`mode: "set"`。
- **参数**：
  - `hw`：目前仅支持 `"motor"`。
  - `value`：`"new"` / `"old"`，分别对应不同型号的偏移电机。
- **响应**：`{ "return": true }`。
- **副作用**：调用成功将立即写入 Flash 持久化。

### `route.mstar`

- **用途**：透传原始数据至 MStar 芯片。
- **参数**：`value` 为 Base64 编码字符串，代表需要透传的二进制帧。
- **响应**：`{ "return": true/false }`。
- **异常**：Base64 解析失败或发送错误 → `Server error`。
- **注意**：该接口无 `mode` 字段，直接在 `params` 中提供 `value`。

### `3d.delay.set`（条件编译）

- **用途**：配置 3D 延迟时间，仅在 `Projector_Model == LP72BSL_NP52SLC_NP52SL` 目标上有效。
- **模式**：`mode: "set"`。
- **参数**：`delay` 浮点数，单位毫秒，固件会保留三位小数。
- **响应**：`{ "return": true }`。
- **异常**：硬件不支持时返回 `Invalid params`。

### `3d.delay.get`（条件编译）

- **用途**：读取当前 3D 延迟。
- **模式**：`mode: "get"`。
- **响应**：`{ "delay": "x.xxx" }`（字符串形式的毫秒值）。
- **异常**：硬件不支持时返回 `Invalid params`。

### `report.standby.en`

- **用途**：XML 待机上报开关。
- **模式与参数**：
  - `mode: "set"`：`state` 布尔型。
  - `mode: "get"`：返回当前状态。
- **响应**：
  - `set`：`{ "return": true }`，同时立即写入 Flash。
  - `get`：`{ "state": true/false }`。

### `info.power.abnormal`

- **用途**：查询异常关机统计。
- **模式**：`mode: "get"`。
- **响应**：
  ```json
  {
    "power_on_count": <uint32>,
    "power_off_count": <uint32>,
    "abnormal_count": <uint32>
  }
  ```
- **算法**：若关机次数少于开机次数，差值视为异常关机计数。

### `info.power.reset`

- **用途**：清零开/关机计数。
- **模式**：`mode: "set"`。
- **参数**：`state` 布尔型，仅当 `true` 时执行清零。
- **响应**：`{ "return": true/false }`（返回调用时传入的布尔值）。

## 附录

### UDP 通信建议

- 默认使用项目配置的远端 IP 与端口，上位机需确保与下位机处于同一网段。
- 建议实现超时机制（≥500 ms），并在超时时重试或提示用户。
- 上位机应记录 `id` 与请求，用于分辨迟到的响应。

### 参数校验提示

- 布尔值必须使用 JSON 原生的 `true`/`false`，不可使用字符串 `"true"`。
- 数字字段应为整数（除 3D 延迟使用浮点），避免字符串数字。
- 当接口返回 `return: false` 时，通常表示 UART 下发失败或外设未应答，可重试或检查硬件连接。

### 版本兼容

- 新增接口将通过固件升级提供，请在使用前调用 `info.version` 判断固件版本。
- 对于条件编译接口（如 3D 延迟），上位机应容忍 `Method not found` 或 `Invalid params`，并按需降级处理。

如需扩展或修订接口，请同步更新本文件，确保上位机开发人员始终获取准确的协议信息。


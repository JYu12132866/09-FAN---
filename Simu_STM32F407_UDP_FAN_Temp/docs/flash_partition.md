# STM32F407ZET6 Flash 分区说明

## 芯片规格

| 项目 | 参数 |
|------|------|
| MCU | STM32F407ZET6 |
| Flash 容量 | 512KB |
| Flash 起始地址 | 0x08000000 |
| SRAM 起始地址 | 0x20000000 |

## 分区表

| Sector | 起始地址 | 大小 | 用途 |
|--------|----------|------|------|
| 0 | 0x08000000 | 16KB | IAP Code (Bootloader) |
| 1 | 0x08004000 | 16KB | IAP Code (Bootloader) |
| 2 | 0x08008000 | 16KB | IAP Code (Bootloader) |
| 3 | 0x0800C000 | 16KB | IAP Param |
| 4 | 0x08010000 | 64KB | APP Code |
| 5 | 0x08020000 | 128KB | APP Code |
| 6 | 0x08040000 | 128KB | APP Code |
| 7 | 0x08060000 | 128KB | APP Param | 

## 分区布局图

```
0x08000000 +------------------+
           |   IAP Code       |  Sector 0-2 (48KB)
0x0800C000 +------------------+
           |   IAP Param      |  Sector 3   (16KB)
0x08010000 +------------------+
           |                  |
           |   APP Code       |  Sector 4-6 (320KB)
           |                  |
0x08060000 +------------------+
           |   APP Param      |  Sector 7   (128KB)
0x08080000 +------------------+
```

## 空间分配统计

| 分区 | 扇区 | 大小 |
|------|------|------|
| Bootloader 代码 | Sector 0-2 | 48KB |
| Bootloader 参数 | Sector 3 | 16KB |
| 应用程序代码 | Sector 4-6 | 320KB |
| 应用程序参数 | Sector 7 | 128KB |
| **总计** | - | **512KB** |

## 相关代码定义

```c
// 分区地址宏定义 (stmflash.h)
#define ADDR_FLASH_SECTOR_0     0x08000000  // 16KB
#define ADDR_FLASH_SECTOR_3     0x0800C000  // 16KB
#define ADDR_FLASH_SECTOR_4     0x08010000  // 64KB
#define ADDR_FLASH_SECTOR_7     0x08060000  // 128KB

// 功能分区地址定义 (iap.c)
#define FLASH_BOOT_CODE_ADDR    ADDR_FLASH_SECTOR_0
#define FLASH_BOOT_PARAM_ADDR   ADDR_FLASH_SECTOR_3
#define FLASH_APP_CODE_ADDR     ADDR_FLASH_SECTOR_4
#define FLASH_APP_PARAM_ADDR    ADDR_FLASH_SECTOR_7
```


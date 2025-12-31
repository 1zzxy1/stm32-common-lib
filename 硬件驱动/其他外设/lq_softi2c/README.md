# LQ_SOFTI2C 软件I2C库

> ✅ **通用模块** - 支持多平台（需GPIO模拟）

## 功能特性

- GPIO模拟I2C通信
- 标准I2C协议
- 支持多字节读写
- 可配置引脚
- 无需硬件I2C外设

## 使用场景

✅ **适用于**：任何有GPIO的单片机
⚠️ **不适用于**：对速度要求极高的场景（推荐硬件I2C）

## API示例

```c
#include "LQ_SOFTI2C.h"

// 初始化
I2C_Init();

// 写数据
I2C_Write_Byte(0xA0, 0x10, data);

// 读数据
data = I2C_Read_Byte(0xA0, 0x10);
```

## 依赖项

GPIO控制函数

## 来源

网友那拿的（龙邱例程）

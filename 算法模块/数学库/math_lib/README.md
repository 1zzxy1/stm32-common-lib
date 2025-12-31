# math_lib 数学工具函数库

> ✅ **通用模块** - 支持所有平台

## 功能特性

- map映射函数（Arduino风格）
- Clamp限幅函数
- 纯C实现，无依赖

## 使用场景

✅ **适用于**：所有需要数值映射和限幅的场景

## API示例

```c
#include "math_lib.h"

// 映射函数（类似Arduino map）
float pwm = map(speed, 0, 100, 0, 10000);
// 将速度0~100映射到PWM值0~10000

// 限幅函数
float angle = Clamp(raw_angle, -45.0, 45.0);
// 将角度限制在-45°~45°范围内
```

## 函数说明

### map函数
```c
float map(float x, float in_min, float in_max, float out_min, float out_max)
```
将x从输入范围[in_min, in_max]线性映射到输出范围[out_min, out_max]

### Clamp函数
```c
float Clamp(float x, float min, float max)
```
将x限制在[min, max]范围内

## 依赖项

无

## 来源

网友那拿的

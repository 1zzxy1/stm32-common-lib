# WP_Math高性能数学库

> ✅ **通用算法模块** - 纯软件库，无硬件依赖

## 功能特性

- **100+优化函数**：三角函数、反三角、指数、对数等
- **AVR级优化**：专为嵌入式平台优化，比标准库快3~10倍
- **完整常数**：WGS84地球参数、数学常数、单位转换
- **约束函数**：各种数值限幅和安全计算

## 使用场景

✅ **适用于**：
- 频繁调用三角函数的算法（姿态解算、导航）
- 对实时性要求高的控制算法
- Flash空间有限但需要数学运算
- 需要地理坐标转换

⚠️ **不适用于**：
- 对精度要求极高的科学计算（精度略低于标准库）
- 运算量很小的场合（优化收益不明显）

## 配置说明

无需配置，直接包含头文件使用：

```c
#include "WP_Math.h"
```

## API示例

### 快速三角函数

```c
// 标准库函数（慢）
float sin_val = sinf(angle);
float cos_val = cosf(angle);

// 优化函数（快3~5倍）
float sin_val = FastSin(angle);
float cos_val = FastCos(angle);

// 同时计算sin和cos（更快）
float sin_val, cos_val;
FastSinCos(angle, &sin_val, &cos_val);
```

### 反三角函数

```c
float angle = FastAsin(value);      // 反正弦
float angle = FastAtan(value);      // 反正切
float angle = FastAtan2(y, x);      // 双参数反正切
```

### 指数和对数

```c
float ln_val = FastLn(x);           // 自然对数
float pow_val = FastPow(base, exp); // 幂运算
```

### 快速平方根

```c
float sqrt_val = FastSqrt(x);       // 快速平方根
float inv_sqrt = FastSqrtI(x);      // 快速反平方根（Quake算法）
```

### 几何运算

```c
// 安全反正弦（自动限幅）
float angle = safe_asin(value);

// 安全平方根（负数返回0）
float result = safe_sqrt(value);

// 勾股定理
float hypotenuse = pythagorous2(a, b);           // sqrt(a²+b²)
float hypotenuse3 = pythagorous3(a, b, c);       // sqrt(a²+b²+c²)
```

### 约束函数

```c
// 浮点数限幅
float limited = constrain_float(value, min, max);

// 整数限幅
int16_t limited = constrain_int16(value, min, max);
int32_t limited = constrain_int32(value, min, max);
```

### 角度和弧度转换

```c
float rad = DEG_TO_RAD * degrees;   // 角度→弧度
float deg = RAD_TO_DEG * radians;   // 弧度→角度
```

### 地理坐标转换

```c
// 纬度差转米/厘米
float meters = latitude_diff * LATLON_TO_M;
float cm = latitude_diff * LATLON_TO_CM;

// 经度差转米（需考虑纬度）
float meters = longitude_diff * cosf(latitude) * LATLON_TO_M;
```

## 主要函数列表

### 三角函数

| 函数 | 功能 | 提速倍数 |
|------|------|----------|
| `FastSin(x)` | 快速正弦 | 3~5x |
| `FastCos(x)` | 快速余弦 | 3~5x |
| `FastTan(x)` | 快速正切 | 3~5x |
| `FastSinCos(x, *s, *c)` | 同时计算sin/cos | 5~8x |

### 反三角函数

| 函数 | 功能 | 提速倍数 |
|------|------|----------|
| `FastAsin(x)` | 快速反正弦 | 3~4x |
| `FastAtan(x)` | 快速反正切 | 3~4x |
| `FastAtan2(y, x)` | 快速双参数反正切 | 3~4x |

### 指数/对数/幂函数

| 函数 | 功能 | 提速倍数 |
|------|------|----------|
| `FastLn(x)` | 快速自然对数 | 4~6x |
| `FastPow(b, e)` | 快速幂运算 | 3~5x |
| `FastSqrt(x)` | 快速平方根 | 2~3x |
| `FastSqrtI(x)` | 快速反平方根 | 5~10x |

### 安全计算函数

| 函数 | 功能 |
|------|------|
| `safe_asin(x)` | 安全反正弦（自动限幅±1） |
| `safe_sqrt(x)` | 安全平方根（负数返回0） |
| `pythagorous2(a, b)` | 二维勾股定理 |
| `pythagorous3(a, b, c)` | 三维勾股定理 |

### 约束函数

| 函数 | 功能 |
|------|------|
| `constrain_float(v, min, max)` | 浮点数限幅 |
| `constrain_int16(v, min, max)` | 16位整数限幅 |
| `constrain_int32(v, min, max)` | 32位整数限幅 |

## 常数定义

### 数学常数

```c
M_PI              // π
M_PI_2            // π/2
M_PI_3            // π/3
M_2PI             // 2π
DEG_TO_RAD        // 0.017453 (π/180)
RAD_TO_DEG        // 57.29578 (180/π)
```

### 地球参数（WGS84）

```c
LATLON_TO_M       // 纬度/经度1°对应米数
LATLON_TO_CM      // 纬度/经度1°对应厘米数
```

## 性能对比

以STM32F4@168MHz测试，100万次调用：

| 函数 | 标准库耗时 | WP_Math耗时 | 提速 |
|------|-----------|-------------|------|
| sinf() | 45ms | 12ms | **3.75x** |
| cosf() | 45ms | 12ms | **3.75x** |
| tanf() | 50ms | 15ms | **3.33x** |
| asinf() | 60ms | 18ms | **3.33x** |
| atanf() | 55ms | 16ms | **3.44x** |
| atan2f() | 65ms | 20ms | **3.25x** |
| logf() | 70ms | 16ms | **4.38x** |
| powf() | 80ms | 22ms | **3.64x** |
| sqrtf() | 30ms | 12ms | **2.50x** |

## 精度说明

| 函数类型 | 误差范围 | 说明 |
|---------|---------|------|
| 三角函数 | <0.001 rad | 适合姿态解算 |
| 反三角函数 | <0.002 rad | 适合姿态解算 |
| 指数/对数 | <0.5% | 适合一般计算 |
| 平方根 | <0.1% | 适合距离计算 |

## 注意事项

1. **精度权衡**：比标准库略低（约0.1%~0.5%误差），但对控制算法影响很小
2. **输入范围**：三角函数输入应为弧度制
3. **线程安全**：所有函数都是纯函数，线程安全
4. **Flash占用**：完整库约10KB Flash（可按需裁剪）

## 使用建议

**推荐使用场景**：
- IMU姿态解算（Madgwick、互补滤波）
- 导航算法（GPS、里程计）
- 机器人运动学计算
- PID控制器（如需三角函数）

**不推荐场景**：
- 金融计算（精度要求高）
- 每秒调用次数<1000次（优化收益不明显）

## 依赖项

- 无依赖
- 纯C实现，跨平台

## 来源

- 武汉匿名创新技术有限公司
- CarryPilot飞控项目

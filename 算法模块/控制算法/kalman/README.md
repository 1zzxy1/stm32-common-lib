# Kalman Filter 卡尔曼滤波器

轻量级一维卡尔曼滤波器实现，适用于传感器数据平滑、噪声滤除等场景。

## 特性

- 标准一维卡尔曼滤波算法
- 可配置过程噪声和测量噪声参数
- 支持运行时调整参数
- 纯C实现，无硬件依赖
- 低内存占用 (约28字节/实例)

## 使用方法

### 1. 基本使用

```c
#include "kalman.h"

// 定义滤波器实例
kalman_t sensor_filter;

// 使用默认参数初始化 (Q=0.01, R=0.1)
kalman_init_default(&sensor_filter, 0.0f);

// 在传感器读取循环中使用
while (1) {
    float raw_value = read_sensor();
    float filtered = kalman_update(&sensor_filter, raw_value);
    // 使用滤波后的值
}
```

### 2. 自定义参数

```c
kalman_t temp_filter;

// 自定义初始化
// 参数: 初始值, 初始协方差, 过程噪声Q, 测量噪声R
kalman_init(&temp_filter, 25.0f, 1.0f, 0.001f, 0.5f);
```

### 3. 多传感器滤波

```c
kalman_t accel_x, accel_y, accel_z;

void init_filters(void)
{
    kalman_init_default(&accel_x, 0.0f);
    kalman_init_default(&accel_y, 0.0f);
    kalman_init_default(&accel_z, 9.8f);  // 重力加速度
}

void filter_accel(float *ax, float *ay, float *az)
{
    *ax = kalman_update(&accel_x, *ax);
    *ay = kalman_update(&accel_y, *ay);
    *az = kalman_update(&accel_z, *az);
}
```

### 4. 运行时调参

```c
// 根据场景动态调整
if (high_speed_mode) {
    // 快速响应模式: 更信任测量值
    kalman_set_r(&sensor_filter, 0.01f);
} else {
    // 平滑模式: 更信任模型预测
    kalman_set_r(&sensor_filter, 1.0f);
}
```

## 参数说明

| 参数 | 说明 | 建议值范围 |
|------|------|-----------|
| `Q` | 过程噪声协方差，越小越信任模型预测 | 0.001 ~ 0.1 |
| `R` | 测量噪声协方差，越小越信任测量值 | 0.01 ~ 1.0 |
| `P` | 初始估计误差协方差 | 1.0 |

### 参数调节技巧

- **测量噪声大** (传感器不稳定): 增大R值
- **需要快速跟踪**: 减小R值或增大Q值
- **需要平滑输出**: 增大R值或减小Q值
- **系统变化快**: 增大Q值

## API 概览

| 函数 | 说明 |
|------|------|
| `kalman_init()` | 自定义参数初始化 |
| `kalman_init_default()` | 使用默认参数初始化 |
| `kalman_update()` | 执行滤波迭代 |
| `kalman_reset()` | 重置滤波器状态 |
| `kalman_set_q()` | 设置过程噪声协方差 |
| `kalman_set_r()` | 设置测量噪声协方差 |
| `kalman_get_state()` | 获取当前估计值 |
| `kalman_get_gain()` | 获取当前卡尔曼增益 |

## 算法原理

卡尔曼滤波包含两个阶段：

**1. 预测阶段**
```
x(k|k-1) = A * x(k-1|k-1)
P(k|k-1) = A * P(k-1|k-1) * A' + Q
```

**2. 更新阶段**
```
K = P(k|k-1) * H' / (H * P(k|k-1) * H' + R)
x(k|k) = x(k|k-1) + K * (z(k) - H * x(k|k-1))
P(k|k) = (I - K * H) * P(k|k-1)
```

对于一维恒定值模型，A=1, H=1，算法得到简化。

## 应用场景

- 温度传感器数据滤波
- 加速度计/陀螺仪数据平滑
- 电压/电流测量滤波
- 距离传感器数据处理
- ADC采样值滤波

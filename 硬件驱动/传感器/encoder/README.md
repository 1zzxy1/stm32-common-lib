# Encoder Driver 编码器驱动库

基于STM32 HAL库的正交编码器驱动库，支持速度测量和位置累计。

## 特性

- 基于定时器编码器模式（4倍频）
- 支持速度测量 (mm/s, cm/s, RPM)
- 支持距离累计
- 支持编码器反向配置
- 可配置编码器参数

## 依赖

- STM32 HAL库 (tim.h)

## 使用方法

### 1. STM32CubeMX配置

1. 选择一个定时器（如TIM2、TIM3等）
2. Combined Channels: Encoder Mode
3. 设置编码器模式：Encoder Mode TI1 and TI2
4. Counter Period: 65535 (16位)

### 2. 配置编码器参数

在包含头文件前定义参数：

```c
// 根据实际编码器和机械结构配置
#define ENCODER_PPR 500             // 编码器线数 * 4
#define ENCODER_GEAR_RATIO 20       // 减速比
#define ENCODER_WHEEL_DIAMETER_MM 65.0f  // 轮子直径(mm)
#define ENCODER_SAMPLE_TIME_MS 10   // 采样周期(ms)

#include "encoder_driver.h"
```

### 3. 定义和初始化编码器

```c
encoder_t left_encoder;
encoder_t right_encoder;

void Encoder_Init(void)
{
    // TIM2用于左编码器，正常方向
    encoder_init(&left_encoder, &htim2, 0);

    // TIM3用于右编码器，反向
    encoder_init(&right_encoder, &htim3, 1);
}
```

### 4. 周期性更新（与采样周期一致）

```c
// 在10ms周期任务中调用
void Encoder_Task(void)
{
    encoder_update(&left_encoder);
    encoder_update(&right_encoder);
}
```

### 5. 获取数据

```c
// 获取速度
float left_speed = encoder_get_speed_mm_s(&left_encoder);   // mm/s
float left_speed_cm = encoder_get_speed_cm_s(&left_encoder); // cm/s
float rpm = encoder_get_speed_rpm(&left_encoder);           // RPM

// 获取位置
int32_t count = encoder_get_total_count(&left_encoder);
float distance = encoder_get_distance_mm(&left_encoder);    // mm

// 重置位置
encoder_reset(&left_encoder);
```

## 配置选项

| 宏定义 | 说明 | 默认值 |
|--------|------|--------|
| `ENCODER_PPR` | 编码器每转脉冲数 (线数*4) | 2000 |
| `ENCODER_GEAR_RATIO` | 减速比 | 1 |
| `ENCODER_WHEEL_DIAMETER_MM` | 轮子直径 (mm) | 65.0 |
| `ENCODER_SAMPLE_TIME_MS` | 采样周期 (ms) | 10 |

### 计算示例

编码器：13线，减速比20:1，4倍频
```
总PPR = 13 * 4 * 20 = 1040
```

## API 概览

| 函数 | 说明 |
|------|------|
| `encoder_init()` | 初始化编码器 |
| `encoder_update()` | 更新编码器数据 |
| `encoder_get_speed_mm_s()` | 获取速度 (mm/s) |
| `encoder_get_speed_cm_s()` | 获取速度 (cm/s) |
| `encoder_get_speed_rpm()` | 获取转速 (RPM) |
| `encoder_get_total_count()` | 获取累计计数 |
| `encoder_get_distance_mm()` | 获取累计距离 (mm) |
| `encoder_reset()` | 重置累计数据 |

## 注意事项

1. `encoder_update()` 调用周期必须与 `ENCODER_SAMPLE_TIME_MS` 一致
2. 编码器方向与电机方向相反时，设置 `reverse=1`
3. 16位定时器在高速时可能溢出，建议采样周期不要太长

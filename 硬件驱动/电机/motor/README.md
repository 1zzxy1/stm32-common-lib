# Motor Driver 电机驱动库

基于STM32 HAL库的电机驱动库，支持A4950、TB6612等H桥驱动芯片。

## 特性

- 支持PWM+方向控制模式
- 支持正反转和停止
- 支持电机反装配置
- 支持死区补偿
- 使能/失能控制
- 速度百分比控制 (-100 ~ +100)

## 依赖

- STM32 HAL库 (tim.h, gpio.h)

## 使用方法

### 1. 硬件连接

```
MCU                    H桥驱动 (A4950)
PWM引脚 (TIM_CHx) ---> IN1
DIR引脚 (GPIO)    ---> IN2
```

### 2. STM32CubeMX配置

1. 配置定时器为PWM模式
2. 配置方向控制引脚为GPIO输出

### 3. 定义电机实例

```c
#include "motor_driver.h"

motor_t left_motor;
motor_t right_motor;
```

### 4. 初始化电机

```c
void Motor_Init(void)
{
    // 左电机: TIM1_CH1, PA0为方向引脚, 正装
    motor_create(&left_motor, &htim1, TIM_CHANNEL_1,
                 GPIOA, GPIO_PIN_0, 0);

    // 右电机: TIM1_CH2, PA1为方向引脚, 反装
    motor_create(&right_motor, &htim1, TIM_CHANNEL_2,
                 GPIOA, GPIO_PIN_1, 1);
}
```

### 5. 控制电机

```c
// 设置速度 (-100 到 +100)
motor_set_speed(&left_motor, 50);    // 正转50%
motor_set_speed(&right_motor, -30);  // 反转30%

// 停止电机
motor_stop(&left_motor);

// 获取状态
motor_state_t state = motor_get_state(&left_motor);

// 使能/失能
motor_enable(&left_motor, 0);  // 失能
motor_enable(&left_motor, 1);  // 使能
```

## 配置选项

在包含头文件前可定义以下宏来修改默认配置：

```c
#define MOTOR_SPEED_MAX         100     // 最大速度
#define MOTOR_SPEED_MIN         -100    // 最小速度
#define MOTOR_PWM_PERIOD        999     // PWM周期 (与TIM ARR一致)
#define MOTOR_MIN_PWM_THRESHOLD 50      // 最小PWM阈值 (死区补偿)
#include "motor_driver.h"
```

## API 概览

| 函数 | 说明 |
|------|------|
| `motor_create()` | 创建电机实例 |
| `motor_set_speed()` | 设置电机速度 |
| `motor_stop()` | 停止电机 |
| `motor_get_state()` | 获取电机状态 |
| `motor_get_speed()` | 获取当前速度 |
| `motor_enable()` | 使能/失能电机 |

## A4950控制逻辑

| IN1 (PWM) | IN2 (DIR) | 状态 |
|-----------|-----------|------|
| 0 | 0 | 停止 |
| PWM | 0 | 正转 |
| PWM | 1 | 反转 |

## 注意事项

1. `MOTOR_PWM_PERIOD` 需与定时器的ARR值一致
2. 反装电机设置 `reverse=1` 可自动反向
3. 死区补偿可防止低速时电机启动不良

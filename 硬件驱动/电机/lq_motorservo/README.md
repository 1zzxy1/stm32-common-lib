# LQ_MotorServo 电机舵机驱动库

> ⚙️ **硬件依赖模块** - 适用于STC16单片机平台

## 功能特性

- 直流电机PWM控制（正反转+速度调节）
- 舵机PWM控制（角度调节）
- 支持多路电机/舵机控制
- PWM频率可配置
- 方向控制简单直观

## 使用场景

✅ **适用于**：
- STC16F系列单片机项目
- 龙邱智能车开发板
- 小车运动控制
- 舵机云台控制
- 双电机差速驱动

⚠️ **不适用于**：
- STM32平台（需要修改PWM和GPIO接口）
- 步进电机控制（推荐使用emm_v5）
- 大功率电机（需要外加电机驱动模块）

## 配置说明

### 平台要求
- 单片机：STC16F系列
- PWM通道：至少2路（电机）+ 1路（舵机）
- 电机驱动：TB6612/L298N等H桥驱动

### 必须修改的代码

需要根据硬件连接修改IO定义和PWM配置：

```c
// 在 LQ_MotorServo.c 中修改
// 电机控制引脚定义
#define MOTOR_A_IN1    P1^0  // 电机A方向1
#define MOTOR_A_IN2    P1^1  // 电机A方向2
#define MOTOR_B_IN1    P1^2  // 电机B方向1
#define MOTOR_B_IN2    P1^3  // 电机B方向2

// PWM通道定义
#define MOTOR_A_PWM    PWM1  // 电机A速度PWM
#define MOTOR_B_PWM    PWM2  // 电机B速度PWM
#define SERVO_PWM      PWM3  // 舵机PWM
```

## API使用示例

### 电机控制

```c
#include "LQ_MotorServo.h"

void main(void)
{
    // 初始化电机
    Motor_Init();

    // 电机A前进，速度50%
    Motor_A_SetSpeed(5000);  // 范围：-10000 ~ 10000

    // 电机B后退，速度30%
    Motor_B_SetSpeed(-3000);

    // 停止所有电机
    Motor_Stop();
}
```

### 舵机控制

```c
#include "LQ_MotorServo.h"

void main(void)
{
    // 初始化舵机
    Servo_Init();

    // 舵机转到中间位置
    Servo_SetAngle(1500);  // 范围：500~2500 (对应0°~180°)

    // 舵机转到左极限
    Servo_SetAngle(500);

    // 舵机转到右极限
    Servo_SetAngle(2500);
}
```

## 性能参数

- PWM频率：10kHz ~ 50kHz（可配置）
- 速度分辨率：10000级（-10000 ~ 10000）
- 舵机精度：0.09°（2000个脉宽级别）
- 响应速度：< 1ms

## 注意事项

1. **平台限制**：专为STC16平台设计，移植到其他平台需要修改底层接口
2. **电源要求**：电机和单片机应分开供电，避免电机启动时拉低单片机电压
3. **PWM频率**：舵机频率通常为50Hz，电机频率可用10kHz~50kHz
4. **方向逻辑**：正值正转，负值反转，注意根据实际接线调整
5. **速度限制**：建议加速度控制，避免电流过大损坏驱动芯片

## 依赖项

- STC16F头文件
- 龙邱STC16开发板底层驱动（LQ_PWM等）

## 来源

网友那拿的（龙邱智能车例程）

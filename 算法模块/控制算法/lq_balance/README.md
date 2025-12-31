# LQ_Balance 平衡车控制算法

> ✅ **通用算法模块** - 适用于STC16单片机平台

## 功能特性

- 直立平衡PID控制
- 速度环PID控制
- 双闭环控制策略
- MPU6050 DMP姿态融合
- 编码器速度反馈
- 按键参数调节

## 使用场景

✅ **适用于**：
- STC16F系列单片机项目
- 龙邱智能车开发板
- 两轮自平衡小车
- 倒立摆实验
- 控制算法学习

⚠️ **不适用于**：
- 四轮麦克纳姆轮车（需要不同的控制策略）
- 高速循线车（响应速度要求更高）
- STM32平台（需要修改底层接口）

## 配置说明

### 硬件要求
- 单片机：STC16F系列
- 传感器：MPU6050（含DMP功能）
- 编码器：2路增量式编码器
- 电机：2路直流减速电机
- 电机驱动：TB6612/L298N

### 算法参数

```c
// 在 LQ_balance.c 中调整PID参数

// 直立环PID参数
float Balance_Kp = 30.0;    // 比例系数
float Balance_Kd = 0.8;     // 微分系数

// 速度环PI参数
float Velocity_Kp = 5.0;    // 比例系数
float Velocity_Ki = 0.1;    // 积分系数

// 目标速度
int Target_Speed = 0;       // 速度设定值
```

## API使用示例

### 初始化和主循环

```c
#include "LQ_balance.h"

void main(void)
{
    // 初始化硬件
    MPU6050_DMP_Init();  // 初始化陀螺仪
    Encoder_Init();      // 初始化编码器
    Motor_Init();        // 初始化电机

    // 启动定时器中断（5ms）
    Timer_Init();

    while(1)
    {
        // 按键调参
        Key_Process();

        // OLED显示
        OLED_Show();

        delay_ms(100);
    }
}

// 定时器中断（5ms）
void Timer_ISR(void) interrupt 1
{
    // 读取姿态角度和角速度
    float angle, gyro;
    MPU6050_DMP_Get_Data(&angle, &gyro);

    // 读取编码器速度
    int encoder_left = Encoder_Get_Left();
    int encoder_right = Encoder_Get_Right();

    // 计算直立环输出
    int balance_pwm = SBB_Get_BalancePID(angle, gyro);

    // 计算速度环输出
    int speed_pwm_left = SBB_Get_MotorPI(encoder_left, Target_Speed);
    int speed_pwm_right = SBB_Get_MotorPI(encoder_right, Target_Speed);

    // 输出到电机
    Motor_Set(balance_pwm + speed_pwm_left, balance_pwm + speed_pwm_right);
}
```

### 参数调节

```c
// 运行时调整PID参数
void Key_Process(void)
{
    if(Key1_Press())
    {
        Balance_Kp += 1.0;  // 增大直立P
    }
    if(Key2_Press())
    {
        Balance_Kp -= 1.0;  // 减小直立P
    }
    if(Key3_Press())
    {
        Target_Speed += 10;  // 增加目标速度
    }
    if(Key4_Press())
    {
        Target_Speed -= 10;  // 减少目标速度
    }
}
```

## 控制原理

### 双闭环控制结构

```
[角度反馈] ──→ [直立环PID] ──┐
                            ├──→ [电机PWM输出]
[速度反馈] ──→ [速度环PI]  ──┘
```

### 直立环（内环）
- 输入：倾角、角速度
- 输出：平衡力矩
- 作用：快速纠正倾倒
- 频率：200Hz

### 速度环（外环）
- 输入：编码器速度
- 输出：速度补偿
- 作用：位置保持、前进后退
- 频率：200Hz

## 调参建议

1. **先调直立环**：
   - 从小到大增加Kp，直到车能短暂站立
   - 增加Kd，抑制震荡，提高稳定性
   - Balance_Kp通常在20~50之间
   - Balance_Kd通常在0.5~2之间

2. **再调速度环**：
   - 先设置Velocity_Kp，让车能响应速度指令
   - 再加入Velocity_Ki，消除稳态误差
   - 速度环参数要比直立环小很多

3. **注意事项**：
   - 机械中心要找准，轮子要对齐
   - 电池电压影响很大，满电和低电PID要不同
   - 调试时小心车冲出去，注意安全

## 性能参数

- 控制频率：200Hz（5ms中断）
- 平衡角度：±30°可恢复
- 响应速度：< 50ms
- 稳态误差：< 2°
- 速度控制精度：±10 encoder/s

## 注意事项

1. **安全第一**：调试时做好保护，防止车冲出去摔坏
2. **中心校准**：机械重心要正，陀螺仪安装要水平
3. **电池电压**：满电和低电特性差异大，需要调整参数
4. **环境因素**：地面摩擦、轮胎气压都会影响效果
5. **代码优化**：中断函数要快，避免影响控制频率

## 依赖项

- STC16F头文件
- LQ_MPU6050_DMP库
- LQ_Encoder编码器驱动
- LQ_MotorServo电机驱动

## 来源

网友那拿的（龙邱智能车例程）

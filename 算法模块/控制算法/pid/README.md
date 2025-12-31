# PID 控制器库

通用PID控制器实现，支持位置式和增量式两种算法。

## 特性

- 支持**位置式PID**和**增量式PID**
- 内置输出限幅功能
- 积分抗饱和支持
- 纯C实现，无硬件依赖
- 适用于任何嵌入式平台

## 使用方法

### 1. 添加文件到工程

将 `pid.c` 和 `pid.h` 添加到你的工程中。

### 2. 基本使用示例

```c
#include "pid.h"

// 定义PID控制器
PID_T speed_pid;

// 初始化 (Kp=1.0, Ki=0.1, Kd=0.5, 目标值=100, 输出限幅=1000)
pid_init(&speed_pid, 1.0f, 0.1f, 0.5f, 100.0f, 1000.0f);

// 在控制循环中调用
void control_loop(void)
{
    float current_speed = get_current_speed();  // 获取当前值

    // 使用增量式PID计算输出
    float output = pid_calculate_incremental(&speed_pid, current_speed);

    // 或使用位置式PID
    // float output = pid_calculate_positional(&speed_pid, current_speed);

    set_motor_pwm(output);  // 应用输出
}
```

### 3. 动态调整参数

```c
// 修改目标值
pid_set_target(&speed_pid, 200.0f);

// 修改PID参数
pid_set_params(&speed_pid, 2.0f, 0.2f, 1.0f);

// 修改输出限幅
pid_set_limit(&speed_pid, 500.0f);

// 重置控制器（清除历史误差）
pid_reset(&speed_pid);
```

## API 说明

| 函数 | 说明 |
|------|------|
| `pid_init()` | 初始化PID控制器 |
| `pid_set_target()` | 设置目标值 |
| `pid_set_params()` | 设置Kp、Ki、Kd参数 |
| `pid_set_limit()` | 设置输出限幅 |
| `pid_reset()` | 重置控制器状态 |
| `pid_calculate_positional()` | 位置式PID计算 |
| `pid_calculate_incremental()` | 增量式PID计算 |
| `pid_constrain()` | 通用限幅函数 |
| `pid_limit_integral()` | 积分限幅（防饱和） |

## 位置式 vs 增量式

| 特性 | 位置式PID | 增量式PID |
|------|----------|----------|
| 输出 | 绝对值 | 增量值 |
| 积分饱和 | 需要处理 | 天然抗饱和 |
| 适用场景 | 位置控制 | 速度控制 |
| 切换平滑 | 可能有跳变 | 平滑过渡 |

## 调参建议

1. 先调P：从小到大，直到系统快速响应但不振荡
2. 再调D：抑制超调，提高稳定性
3. 最后调I：消除稳态误差

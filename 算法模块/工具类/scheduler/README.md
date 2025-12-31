# Scheduler 任务调度器

轻量级任务调度器，基于时间片的非抢占式调度，适用于嵌入式裸机系统。

## 特性

- 基于时间片的非抢占式调度
- 支持动态添加/移除任务
- 支持任务使能/禁用
- 支持运行时修改任务周期
- 纯C实现，无硬件依赖
- 低内存占用

## 使用方法

### 1. 定义调度器

```c
#include "scheduler.h"

// 定义调度器实例
scheduler_t my_scheduler;
```

### 2. 定义任务函数

```c
void led_task(void)
{
    // LED闪烁逻辑
    HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
}

void sensor_task(void)
{
    // 传感器读取逻辑
}

void control_task(void)
{
    // 控制逻辑
}
```

### 3. 初始化和添加任务

```c
void main(void)
{
    // 初始化硬件...

    // 初始化调度器
    scheduler_init(&my_scheduler);

    // 添加任务 (函数, 周期ms)
    int led_idx = scheduler_add_task(&my_scheduler, led_task, 500);       // 500ms周期
    int sensor_idx = scheduler_add_task(&my_scheduler, sensor_task, 10);  // 10ms周期
    int ctrl_idx = scheduler_add_task(&my_scheduler, control_task, 20);   // 20ms周期

    // 主循环
    while (1) {
        scheduler_run(&my_scheduler, HAL_GetTick());
    }
}
```

### 4. 运行时控制

```c
// 禁用任务
scheduler_enable_task(&my_scheduler, led_idx, 0);

// 重新使能任务
scheduler_enable_task(&my_scheduler, led_idx, 1);

// 修改任务周期
scheduler_set_period(&my_scheduler, sensor_idx, 50);  // 改为50ms

// 移除任务
scheduler_remove_task(&my_scheduler, ctrl_idx);
```

## 配置选项

```c
// 在包含头文件前定义，修改最大任务数
#define SCHEDULER_MAX_TASKS 32
#include "scheduler.h"
```

## API 概览

| 函数 | 说明 |
|------|------|
| `scheduler_init()` | 初始化调度器 |
| `scheduler_add_task()` | 添加任务 |
| `scheduler_remove_task()` | 移除任务 |
| `scheduler_enable_task()` | 使能/禁用任务 |
| `scheduler_set_period()` | 修改任务周期 |
| `scheduler_run()` | 运行调度器（主循环调用） |
| `scheduler_get_task_count()` | 获取当前任务数量 |

## 注意事项

1. 任务函数应尽快返回，避免阻塞其他任务
2. 对于需要精确定时的任务，建议使用硬件定时器中断
3. `scheduler_run()` 需要传入准确的系统时间（毫秒）
4. 任务周期精度取决于主循环调用 `scheduler_run()` 的频率

# HWT101陀螺仪驱动

> ⚙️ **硬件依赖模块** - UART串口通信

## 功能特性

- **串口陀螺仪**：输出角速度和偏航角
- **高精度**：静态精度±0.5°
- **可配置**：波特率、输出频率、校准
- **稳定可靠**：工业级设计

## 使用场景

✅ **适用于**：
- 航向角测量（车辆导航、机器人定位）
- 角速度监测（旋转控制）
- 需要Z轴角度的场合（单轴稳定）

⚠️ **不适用于**：
- 需要完整姿态（Roll/Pitch/Yaw）的场景（推荐JY901S）
- 高频率采样（>100Hz）

## 配置说明

### 代码配置

```c
// 在hwt101_driver.h中修改
extern UART_HandleTypeDef huart1;
#define HWT101_UART_HANDLE huart1

// UART接收回调中处理数据
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if(huart == &huart1) {
        HWT101_ProcessBuffer(rx_buffer, rx_len);
    }
}
```

## API示例

```c
#include "hwt101_driver.h"

HWT101_t gyro;

// 初始化
HWT101_Create(&gyro);

// 处理串口数据
HWT101_ProcessBuffer(&gyro, uart_data, len);

// 读取数据
float yaw = HWT101_GetYaw(&gyro);
float gyro_z = HWT101_GetGyroZ(&gyro);

// 配置参数
HWT101_SetBaudRate(&gyro, HWT101_BAUD_115200);
HWT101_SetOutputRate(&gyro, HWT101_RATE_100HZ);

// 校准
HWT101_StartManualCalibration(&gyro);  // 静止放置3秒
HWT101_ZeroYawAngle(&gyro);  // Z轴角度归零
```

## 主要API

| 函数 | 功能 |
|------|------|
| `HWT101_Create(gyro)` | 初始化实例 |
| `HWT101_ProcessBuffer(gyro, data, len)` | 处理接收数据 |
| `HWT101_GetYaw(gyro)` | 读取偏航角（°） |
| `HWT101_GetGyroZ(gyro)` | 读取Z轴角速度（°/s） |
| `HWT101_SetBaudRate(gyro, baud)` | 配置波特率 |
| `HWT101_SetOutputRate(gyro, rate)` | 配置输出频率 |
| `HWT101_ZeroYawAngle(gyro)` | Z轴归零 |
| `HWT101_StartManualCalibration(gyro)` | 手动校准 |

## 波特率选项

```c
HWT101_BAUD_4800
HWT101_BAUD_9600     // 默认
HWT101_BAUD_19200
HWT101_BAUD_38400
HWT101_BAUD_57600
HWT101_BAUD_115200   // 推荐
HWT101_BAUD_230400
```

## 输出频率选项

```c
HWT101_RATE_0_2HZ    // 0.2Hz
HWT101_RATE_0_5HZ
HWT101_RATE_1HZ
HWT101_RATE_2HZ
HWT101_RATE_5HZ
HWT101_RATE_10HZ     // 默认
HWT101_RATE_20HZ
HWT101_RATE_50HZ
HWT101_RATE_100HZ
HWT101_RATE_200HZ
```

## 注意事项

1. 首次使用需校准，静止放置时执行校准
2. 需定期归零避免累积误差
3. 上电后需预热1分钟达到最佳精度
4. 强磁场环境可能影响精度

## 依赖项

- STM32 HAL UART库

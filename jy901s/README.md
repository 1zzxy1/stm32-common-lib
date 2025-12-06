# JY901S九轴IMU驱动

> ⚙️ **硬件依赖模块** - UART串口通信

## 功能特性

- **九轴传感器**：加速度+陀螺仪+磁力计
- **直接输出**：角度、四元数、加速度、角速度、磁场
- **出厂校准**：无需复杂姿态解算
- **高刷新率**：最高200Hz输出
- **低成本**：价格约20-30元

## 使用场景

✅ **适用于**：
- 快速原型开发（无需姿态解算代码）
- 平衡车、自稳云台
- 简单的姿态测量
- 预算有限的项目

⚠️ **不适用于**：
- 高精度姿态控制（推荐BNO08X）
- 需要自定义融合算法的场景

## 配置说明

### 硬件连接

```
STM32        JY901S
---------------------------
PA9 (TX)  -> RX
PA10 (RX) <- TX
3.3V      -> VCC
GND       -> GND
```

### 代码配置

```c
// 在jy901s.h中修改UART句柄
extern UART_HandleTypeDef huart1;  // 改为实际使用的UART
#define JY901S_UART huart1
```

## API示例

```c
#include "jy901s.h"

// 初始化
JY901S_Init();

// 主循环读取
while(1) {
    float roll, pitch, yaw;
    JY901S_GetAngles(&roll, &pitch, &yaw);

    printf("Roll: %.2f, Pitch: %.2f, Yaw: %.2f\n", roll, pitch, yaw);
    HAL_Delay(10);
}
```

## 主要API

| 函数 | 功能 |
|------|------|
| `JY901S_Init()` | 初始化传感器 |
| `JY901S_GetAngles(r,p,y)` | 读取欧拉角（°） |
| `JY901S_GetQuaternion(...)` | 读取四元数 |
| `JY901S_GetAcceleration(x,y,z)` | 读取加速度(m/s²) |
| `JY901S_GetGyro(x,y,z)` | 读取角速度(°/s) |

## 性能参数

- 角度精度：±0.05°（静态）
- 刷新率：0.2Hz ~ 200Hz可配
- 波特率：9600 ~ 921600 bps
- 功耗：约35mA@3.3V

## 注意事项

1. 默认波特率9600，可通过上位机配置
2. 上电后自动输出数据，无需主动请求
3. 磁力计易受干扰，需要校准
4. 串口接收建议使用DMA+中断模式

## 依赖项

- STM32 HAL UART库

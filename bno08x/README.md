# BNO08X九轴IMU驱动

> ⚙️ **硬件依赖模块** - 需要配置UART/I2C通信接口

## 功能特性

BNO08X是Hillcrest Labs推出的高精度9轴惯性测量单元（IMU），内置传感器融合协处理器。

- **九轴传感器**：3轴加速度计 + 3轴陀螺仪 + 3轴磁力计
- **内置融合**：硬件姿态融合处理器（SH-2协议）
- **高精度**：动态精度±1°，静态精度±0.5°
- **多种输出**：四元数、欧拉角、旋转矩阵、线性加速度
- **低功耗**：融合模式下功耗<10mA
- **快速响应**：最高输出频率400Hz

## 使用场景

✅ **适用于**：
- 高精度姿态测量（无人机、云台、机器人）
- 运动追踪和手势识别
- 室内导航和定位系统
- VR/AR设备姿态跟踪
- 平衡车、自稳定平台

⚠️ **不适用于**：
- 低成本入门项目（推荐MPU6050）
- 简单角度测量（推荐HWT101）

## 硬件要求

### 通信接口

支持两种通信方式（二选一）：

**方式1：UART串口**（推荐）
- 波特率：115200 ~ 3000000 bps（默认115200）
- 接口：TX、RX、VCC、GND
- 优点：实现简单，稳定可靠

**方式2：I2C总线**
- 地址：0x4A或0x4B（SA0引脚配置）
- 速度：标准100kHz / 快速400kHz
- 优点：节省引脚，支持多设备

### 硬件复位（可选）

- **RESET引脚**：低电平复位，需配置GPIO输出
- 复位时序：拉低>10ms，释放后等待>100ms

### 中断引脚（可选）

- **INT引脚**：数据就绪中断，可配置GPIO输入+EXTI
- 优势：减少轮询，降低CPU占用

## 配置说明

### 必须修改的代码

#### 1. 选择通信接口

在 `bno08x_hal.h` 中配置：

```c
// 方式1：使用UART（推荐）
#define BNO08X_USE_UART
extern UART_HandleTypeDef huart1;  // 修改为实际使用的UART
#define BNO08X_UART_HANDLE huart1

// 方式2：使用I2C
// #define BNO08X_USE_I2C
// extern I2C_HandleTypeDef hi2c1;
// #define BNO08X_I2C_HANDLE hi2c1
// #define BNO08X_I2C_ADDR 0x4A  // 或0x4B
```

#### 2. 配置复位引脚（可选）

```c
// 如果使用硬件复位功能
#define BNO08X_RST_PIN       GPIO_PIN_0
#define BNO08X_RST_PORT      GPIOA
```

## API使用示例

### 基本初始化

```c
#include "bno08x_hal.h"

// 1. 初始化BNO08X
if (BNO08X_Init() != 0) {
    printf("BNO08X初始化失败\n");
    return -1;
}

// 2. 启用姿态输出（100Hz更新）
BNO08X_EnableRotationVector(10000);  // 单位：微秒（10ms = 100Hz）

// 3. 主循环中读取数据
while(1) {
    if (BNO08X_DataAvailable()) {
        float roll, pitch, yaw;
        BNO08X_GetEulerAngles(&roll, &pitch, &yaw);

        printf("Roll: %.2f, Pitch: %.2f, Yaw: %.2f\n",
               roll, pitch, yaw);
    }
    HAL_Delay(10);
}
```

### 四元数输出

```c
// 启用旋转向量（四元数）
BNO08X_EnableRotationVector(10000);  // 100Hz

// 读取四元数
float quat_i, quat_j, quat_k, quat_real;
if (BNO08X_GetQuaternion(&quat_i, &quat_j, &quat_k, &quat_real)) {
    // 使用四元数进行姿态计算
}
```

### 加速度和角速度

```c
// 启用加速度计（200Hz）
BNO08X_EnableAccelerometer(5000);

// 启用陀螺仪（200Hz）
BNO08X_EnableGyro(5000);

// 读取数据
float accel_x, accel_y, accel_z;
float gyro_x, gyro_y, gyro_z;

BNO08X_GetAcceleration(&accel_x, &accel_y, &accel_z);
BNO08X_GetAngularVelocity(&gyro_x, &gyro_y, &gyro_z);
```

### 磁力计校准

```c
// 启动磁力计校准
BNO08X_StartMagnetometerCalibration();

// 校准过程：绕三个轴旋转设备（8字形）
// 等待校准完成...

// 保存校准数据
BNO08X_SaveCalibration();
```

## 主要API函数

| 函数 | 功能 | 返回值 |
|------|------|--------|
| `BNO08X_Init()` | 初始化传感器 | 0=成功 |
| `BNO08X_DataAvailable()` | 检查是否有新数据 | 1=有数据 |
| `BNO08X_EnableRotationVector(us)` | 启用姿态输出 | 0=成功 |
| `BNO08X_GetEulerAngles(r,p,y)` | 读取欧拉角（°） | void |
| `BNO08X_GetQuaternion(...)` | 读取四元数 | 1=成功 |
| `BNO08X_EnableAccelerometer(us)` | 启用加速度计 | 0=成功 |
| `BNO08X_EnableGyro(us)` | 启用陀螺仪 | 0=成功 |
| `BNO08X_HardReset()` | 硬件复位 | void |

## 性能参数

| 参数 | 规格 |
|------|------|
| 姿态精度（动态） | ±1° |
| 姿态精度（静态） | ±0.5° |
| 陀螺仪范围 | ±2000 dps |
| 加速度计范围 | ±16g |
| 磁力计范围 | ±4900μT |
| 最大输出频率 | 400Hz |
| 功耗（融合模式） | <10mA |
| 工作电压 | 2.4~3.6V |

## 注意事项

1. **通信速率**：UART模式下，建议波特率≥115200 bps
2. **上电时序**：上电后需等待>100ms再初始化
3. **磁力计干扰**：附近有强磁场会影响航向精度
4. **温度漂移**：传感器预热5分钟后精度最佳
5. **DMA接收**：UART模式建议使用DMA接收提高效率

## 依赖项

- **STM32 HAL库**：UART_HandleTypeDef 或 I2C_HandleTypeDef
- **无需额外库**：不依赖其他第三方库

## 引脚连接示例

### UART模式（推荐）

```
STM32         BNO08X
---------------------------
PA9 (TX)  ->  RX
PA10 (RX) <-  TX
3.3V      ->  VCC
GND       ->  GND
PA0 (可选)->  RST (复位)
```

### I2C模式

```
STM32         BNO08X
---------------------------
PB8 (SCL) <-> SCL
PB9 (SDA) <-> SDA
3.3V      ->  VCC
GND       ->  GND
GND/3.3V  ->  SA0 (地址选择)
```

## 常见问题

**Q: 初始化失败怎么办？**
A:
1. 检查通信接线是否正确
2. 确认波特率配置一致
3. 尝试硬件复位后重新初始化

**Q: 姿态数据跳变？**
A:
1. 检查磁力计是否受干扰
2. 执行磁力计校准
3. 增加数据滤波

**Q: 与MPU6050的区别？**
A:
- BNO08X内置融合算法，无需外部解算
- 精度更高，但价格更贵（~50元 vs ~10元）
- 功耗更低，适合电池供电设备

## 参考资料

- [BNO080/BNO085数据手册](https://www.ceva-dsp.com/wp-content/uploads/2019/10/BNO080_085-Datasheet.pdf)
- [SH-2通信协议](https://www.ceva-dsp.com/wp-content/uploads/2019/10/BNO080_085-Sensor-Hub-Transport-Protocol.pdf)

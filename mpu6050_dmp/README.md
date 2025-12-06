# MPU6050完整驱动（含DMP）

> ⚙️ **硬件依赖模块** - I2C总线通信

## 功能特性

- **六轴传感器**：3轴加速度计 + 3轴陀螺仪
- **DMP姿态解算**：内置数字运动处理器
- **高性价比**：价格约10元
- **成熟稳定**：应用最广泛的IMU模块

## 使用场景

✅ **适用于**：
- 入门级姿态测量
- 平衡车、自稳云台
- 手势识别、计步器
- 教学和原型开发

⚠️ **不适用于**：
- 需要磁力计的航向测量（推荐MPU9250）
- 高精度姿态（推荐BNO08X）

## 硬件要求

### I2C连接

```
STM32        MPU6050
---------------------------
PB8 (SCL) <- SCL
PB9 (SDA) <- SDA
3.3V      -> VCC
GND       -> GND
```

### I2C地址

- AD0接GND：0x68（默认）
- AD0接VCC：0x69

## 配置说明

```c
// 在mpu6050.h中修改I2C句柄
extern I2C_HandleTypeDef hi2c1;
#define MPU6050_I2C hi2c1

// 配置I2C地址
#define MPU6050_ADDR 0x68  // 或0x69
```

## API示例

### 基本使用（无DMP）

```c
#include "mpu6050.h"

// 初始化
if(MPU_Init() != 0) {
    printf("MPU6050 init failed\n");
}

// 读取原始数据
int16_t accel_x, accel_y, accel_z;
int16_t gyro_x, gyro_y, gyro_z;
int16_t temp;

MPU_Get_Accelerometer(&accel_x, &accel_y, &accel_z);
MPU_Get_Gyroscope(&gyro_x, &gyro_y, &gyro_z);
MPU_Get_Temperature(&temp);
```

### 使用DMP姿态解算

```c
#include "inv_mpu.h"
#include "inv_mpu_dmp_motion_driver.h"

// 初始化DMP
mpu_init();
mpu_set_sensors(INV_XYZ_GYRO | INV_XYZ_ACCEL);
mpu_configure_fifo(INV_XYZ_GYRO | INV_XYZ_ACCEL);
mpu_set_sample_rate(100);  // 100Hz
dmp_load_motion_driver_firmware();
dmp_enable_feature(DMP_FEATURE_6X_LP_QUAT | DMP_FEATURE_SEND_RAW_ACCEL | DMP_FEATURE_SEND_CAL_GYRO);
dmp_set_fifo_rate(100);
mpu_set_dmp_state(1);

// 读取姿态
while(1) {
    if(mpu_dmp_get_data(&pitch, &roll, &yaw) == 0) {
        printf("Pitch: %.2f, Roll: %.2f, Yaw: %.2f\n", pitch, roll, yaw);
    }
    HAL_Delay(10);
}
```

## 主要API

### 基础API

| 函数 | 功能 |
|------|------|
| `MPU_Init()` | 初始化MPU6050 |
| `MPU_Get_Accelerometer(x,y,z)` | 读取加速度 |
| `MPU_Get_Gyroscope(x,y,z)` | 读取角速度 |
| `MPU_Get_Temperature(t)` | 读取温度 |

### DMP API

| 函数 | 功能 |
|------|------|
| `mpu_init()` | 初始化DMP |
| `dmp_load_motion_driver_firmware()` | 加载DMP固件 |
| `dmp_enable_feature(mask)` | 启用DMP功能 |
| `mpu_dmp_get_data(p,r,y)` | 读取姿态角 |

## 性能参数

| 参数 | 规格 |
|------|------|
| 陀螺仪范围 | ±250/500/1000/2000 °/s |
| 加速度范围 | ±2/4/8/16 g |
| 姿态精度（DMP） | ±2°（动态） |
| 输出频率 | 4Hz ~ 8kHz |
| I2C速率 | 100/400 kHz |
| 功耗 | 3.9mA（典型） |

## 注意事项

1. **DMP固件较大**：需要约14KB Flash空间
2. **温漂**：预热5分钟后精度最佳
3. **无磁力计**：Yaw角会漂移，只适合短时间姿态测量
4. **I2C速度**：建议使用400kHz快速模式
5. **中断引脚**：可配置INT引脚用于数据就绪中断

## 依赖项

- STM32 HAL I2C库
- 无需额外库（DMP固件已内置）

## 文件说明

- `mpu6050.c/.h` - 基础驱动
- `inv_mpu.c/.h` - DMP底层接口
- `inv_mpu_dmp_motion_driver.c/.h` - DMP姿态解算
- `dmpKey.h` - DMP固件密钥
- `dmpmap.h` - DMP寄存器映射
- `IIC.c/.h` - I2C硬件抽象层

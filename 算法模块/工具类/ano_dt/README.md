# ANO_DT 匿名地面站通信协议

> ✅ **通用模块** - 支持多平台（STM32/STC16/Arduino等）

## 功能特性

- 匿名地面站V4/V7通信协议
- 传感器数据上传（加速度、陀螺仪、角度等）
- 波形数据发送（最多10路）
- 参数读写功能
- PID参数调试
- 校验和验证

## 使用场景

✅ **适用于**：
- 飞控调试
- 平衡车参数调节
- 传感器数据可视化
- 实时波形监��
- PID参数在线调试

⚠️ **不适用于**：
- 其他地面站软件（仅支持匿名地面站）
- 高频数据传输（建议< 100Hz）

## 配置说明

### 通信参数
- 波特率：115200
- 数据位：8
- 停止位：1
- 校验位：无

### 必须修改的代码

需要实现串口发送函数：

```c
// 在 ANO_DT.c 中实现
void ANO_DT_Send_Byte(uint8_t data)
{
    // 使用你的串口发送函数
    UART_SendByte(data);  // STM32: HAL_UART_Transmit()
}
```

## API使用示例

### 发送传感器数据

```c
#include "ANO_DT.h"

void main(void)
{
    // 初始化串口
    UART_Init(115200);

    while(1)
    {
        // 读取传感器数据
        short ax, ay, az;  // 加速度
        short gx, gy, gz;  // 陀螺仪
        ICM_Get_Raw_data(&ax, &ay, &az, &gx, &gy, &gz);

        // 发送加速度和陀螺仪数据
        ANO_DT_Send_Senser(ax, ay, az, gx, gy, gz, 0, 0, 0);

        delay_ms(20);  // 50Hz
    }
}
```

### 发送姿态角度

```c
void send_attitude(void)
{
    short roll, pitch, yaw;  // 横滚、俯仰、偏航

    // 获取姿态角
    MPU6050_DMP_Get_Euler(&roll, &pitch, &yaw);

    // 发送姿态数据
    ANO_DT_Send_Status(roll, pitch, yaw, 0, 0, 1);
}
```

### 发送波形数据

```c
void send_waveform(void)
{
    int16_t data[10];

    // 准备波形数据
    data[0] = motor_pwm_left;
    data[1] = motor_pwm_right;
    data[2] = encoder_left;
    data[3] = encoder_right;
    data[4] = balance_angle;
    data[5] = target_speed;

    // 发送波形数据（最多10路）
    ANO_DT_Send_Data(data, 6);
}
```

### PID参数调试

```c
// 接收地面站发来的PID参数
void ANO_DT_Receive_Handler(void)
{
    if(ANO_DT_Data_Receive_Anl())
    {
        // 读取PID参数
        Balance_Kp = ANO_DT_Get_PID_P(1);
        Balance_Kd = ANO_DT_Get_PID_D(1);
        Velocity_Kp = ANO_DT_Get_PID_P(2);
        Velocity_Ki = ANO_DT_Get_PID_I(2);
    }
}
```

## 数据帧格式

### 帧头
```
0xAA 0xAA + 功能字 + 长度 + 数据 + 校验和
```

### 常用功能字
- 0x01：加速度和陀螺仪
- 0x02：姿态角度
- 0x03：传感器状态
- 0xF1：波形数据（10路）
- 0x10~0x1F：PID参数

## 匿名地面站设置

1. 下载匿名地面站V7软件
2. 选择对应串口，波特率115200
3. 点击"打开串口"
4. 在"数据监控"页面查看实时数据
5. 在"波形显示"页面查看波形
6. 在"参数调试"页面调整PID

## 性能参数

- 最大发送频率：200Hz（建议50~100Hz）
- 波形通道数：10路
- 数据分辨率：16位有符号整数
- 延迟：< 20ms

## 注意事项

1. **波特率匹配**：单片机和地面站波特率必须一致
2. **发送频率**：不要过快，避免数据丢失
3. **校验和**：协议自带校验，一般不会出错
4. **字节序**：小端模式（低字节在前）
5. **平台兼容**：只需修改串口发送函数即可移植

## 依赖项

- 串口发送函数（需自行实现）

## 来源

网友那拿的（匿名地面站开源协议）

# STM32 通用库

适用于STM32嵌入式开发的通用组件库集合，涵盖控制算法、数据结构、外设驱动等常用功能。

## 库列表

| 库名 | 说明 | 依赖 |
|------|------|------|
| [pid](./pid) | PID控制器，支持位置式和增量式算法 | 无 |
| [ringbuffer](./ringbuffer) | 环形缓冲区，适用于串口等数据收发 | 无 |
| [scheduler](./scheduler) | 任务调度器，基于时间片的非抢占式调度 | 无 |
| [kalman](./kalman) | 卡尔曼滤波器，一维信号滤波 | 无 |
| [bit_array](./bit_array) | 位数组操作库，Header-only | 无 |
| [ebtn](./ebtn) | 按键驱动库，支持组合键和多击检测 | bit_array |
| [motor](./motor) | 电机驱动库，支持H桥驱动芯片 | STM32 HAL |
| [encoder](./encoder) | 编码器驱动库，支持速度测量和位置累计 | STM32 HAL |
| [ad9833](./ad9833) | AD9833 DDS信号发生器驱动 | STM32 HAL |

## 目录结构

```
stm32通用库/
├── pid/                    # PID控制库
│   ├── pid.c
│   ├── pid.h
│   └── README.md
├── ringbuffer/             # 环形缓冲区
│   ├── ringbuffer.c
│   ├── ringbuffer.h
│   └── README.md
├── scheduler/              # 任务调度器
│   ├── scheduler.c
│   ├── scheduler.h
│   └── README.md
├── kalman/                 # 卡尔曼滤波器
│   ├── kalman.c
│   ├── kalman.h
│   └── README.md
├── bit_array/              # 位数组库
│   ├── bit_array.h
│   └── README.md
├── ebtn/                   # 按键驱动库
│   ├── ebtn.c
│   ├── ebtn.h
│   ├── bit_array.h
│   └── README.md
├── motor/                  # 电机驱动库
│   ├── motor_driver.c
│   ├── motor_driver.h
│   └── README.md
├── encoder/                # 编码器驱动库
│   ├── encoder_driver.c
│   ├── encoder_driver.h
│   └── README.md
├── ad9833/                 # AD9833 DDS驱动库
│   ├── ad9833.c
│   ├── ad9833.h
│   └── README.md
├── LICENSE
└── README.md
```

## 使用方法

1. 将需要的库文件夹复制到你的工程目录
2. 在IDE中添加源文件和头文件路径
3. 参考各库的README文档进行配置和使用

### Keil MDK

1. 将库文件添加到工程
2. 在 Options -> C/C++ -> Include Paths 中添加头文件路径

### STM32CubeIDE

1. 将库文件夹复制到工程目录
2. 右键工程 -> Properties -> C/C++ Build -> Settings -> Include paths

## 许可证

- `ringbuffer` 库基于 RT-Thread，采用 Apache-2.0 许可证
- 其他库采用 MIT 许可证

详见 [LICENSE](./LICENSE)

## 贡献

欢迎提交Issue和Pull Request！

# STM32 通用库

> **⚠️ 个人自用项目** - 本仓库为个人学习和项目开发过程中整理的STM32组件库，代码质量和文档仅供参考，不保证生产环境可用性。

适用于STM32嵌入式开发的通用组件库集合，涵盖控制算法、数据结构、外设驱动等常用功能。

## 库列表

### 📌 标识说明

- ✅ **通用模块** - 无硬件依赖，可直接使用
- ⚙️ **硬件依赖** - 需要根据实际硬件修改GPIO/外设配置
- 📦 **第三方库** - 开源第三方组件
- 🔧 **应用模块** - 需要特定库组合或项目配置

### ✅ 通用算法模块

| 库名 | 说明 | 依赖 |
|------|------|------|
| [pid](./pid) | PID控制器，支持位置式和增量式算法 | 无 |
| [ringbuffer](./ringbuffer) | 环形缓冲区，适用于串口等数据收发 | 无 |
| [scheduler](./scheduler) | 任务调度器，基于时间片的非抢占式调度 | 无 |
| [kalman](./kalman) | 卡尔曼滤波器，一维信号滤波 | 无 |
| [bit_array](./bit_array) | 位数组操作库，Header-only | 无 |
| [usart_pack](./usart_pack) | 串口数据包协议，支持多类型打包/解包 | 无 |
| [fft](./fft) | FFT频谱分析，支持THD/SINAD测量 | CMSIS-DSP |
| [imu_fusion](./imu_fusion) | IMU九轴融合算法（Madgwick+Kalman） | wp_math(可选) |
| [wp_math](./wp_math) | 高性能数学库，100+优化函数（3~10倍提速） | 无 |
| [multi_timer](./multi_timer) | 软件定时器管理器（无需RTOS） | 无 |

### ⚙️ 硬件驱动模块（需修改配置）

#### 传感器驱动

| 库名 | 说明 | 依赖 | 配置要求 |
|------|------|------|----------|
| [bno08x](./bno08x) | BNO08X九轴IMU驱动（高精度±0.5°） | STM32 HAL | **必须修改UART/I2C接口** |
| [jy901s](./jy901s) | JY901S九轴IMU驱动（UART直出角度） | STM32 HAL | **必须修改UART接口** |
| [hwt101](./hwt101) | HWT101陀螺仪驱动（带校准） | STM32 HAL | **必须修改UART接口** |
| [mpu6050_dmp](./mpu6050_dmp) | MPU6050完整驱动（含DMP固件） | STM32 HAL | **必须修改I2C接口** |
| [grayscale](./grayscale) | 8通道灰度传感器驱动（I2C） | STM32 HAL | **必须修改I2C接口** |
| [encoder](./encoder) | 编码器驱动库，支持速度测量和位置累计 | STM32 HAL | 修改定时器配置 |

#### 电机驱动

| 库名 | 说明 | 依赖 | 配置要求 |
|------|------|------|----------|
| [motor](./motor) | 电机驱动库，支持H桥驱动芯片 | STM32 HAL | 修改PWM定时器配置 |
| [emm_v5](./emm_v5) | Emm_V5步进电机串口协议驱动 | STM32 HAL | **必须修改UART接口** |
| [tb6612](./tb6612) | TB6612FNG双路DC电机驱动 | STM32 HAL | **必须修改PWM和GPIO配置** |
| [a4950](./a4950) | A4950双路DC电机驱动 | STM32 HAL | **必须修改PWM和GPIO配置** |

#### 其他外设

| 库名 | 说明 | 依赖 | 配置要求 |
|------|------|------|----------|
| [ebtn](./ebtn) | 按键驱动库，支持组合键和多击检测 | bit_array | 无硬件依赖 |
| [ad9833](./ad9833) | AD9833 DDS信号发生器驱动 | STM32 HAL | 修改SPI和GPIO配置 |
| [spi_flash](./spi_flash) | ⚠️ GD25Qxx SPI Flash驱动 | STM32 HAL | **必须修改CS引脚和SPI句柄** |
| [oled](./oled) | ⚠️ SSD1306 OLED显示驱动（基础） | STM32 HAL | **必须修改I2C/SPI接口** |
| [waveform_gen](./waveform_gen) | ⚠️ 波形发生器（DAC+DMA+Timer） | STM32 HAL | **必须修改DAC/Timer/DMA句柄** |
| [maixcam](./maixcam) | MaixCam视觉传感器串口协议解析 | STM32 HAL | **必须修改UART接口** |

### 📦 第三方开源库

| 库名 | 说明 | 依赖 | 注意事项 |
|------|------|------|----------|
| [lfs](./lfs) | LittleFS嵌入式文件系统 | spi_flash | 需实现硬件读写接口 |
| [u8g2](./u8g2) | 功能强大的单色图形库 | I2C/SPI | **字体库37MB，需裁剪** |
| [WouoUI](./WouoUI) | OLED菜单UI框架 | u8g2, ebtn | 需要u8g2和按键驱动 |

### 🔧 应用层模块（特定场景）

| 库名 | 说明 | 依赖 | 使用场景 |
|------|------|------|----------|
| [shell](./shell) | 🔧 LittleFS命令行Shell | lfs, spi_flash | 文件系统调试 |
| [waveform_analyzer](./waveform_analyzer) | 🔧 波形分析器（FFT+谐波） | fft, CMSIS-DSP | **需实现采样率函数** |
| [pid_tuner](./pid_tuner) | 🔧 串口命令行PID调参工具 | usart_pack | 实时PID参数调优 |

### 📦 Python工具库（上位机）

| 库名 | 说明 | 依赖 | 适用场景 |
|------|------|------|----------|
| [simple_uart](./simple_uart) | 完整的Python UART管理库 | pyserial | 上位机串口通信、数据采集 |
| [perspective_transform](./perspective_transform) | 图像透视变换工具 | OpenCV, NumPy | 视觉伺服、图像校正 |

## 目录结构

```
stm32通用库/
├── # ✅ 通用算法模块
├── pid/                    # PID控制库
├── ringbuffer/             # 环形缓冲区
├── scheduler/              # 任务调度器
├── kalman/                 # 卡尔曼滤波器
├── bit_array/              # 位数组库
├── usart_pack/             # 串口数据包协议
├── fft/                    # FFT频谱分析
├── imu_fusion/             # IMU九轴融合算法
├── wp_math/                # 高性能数学库
├── multi_timer/            # 软件定时器管理器
│
├── # ⚙️ 硬件驱动模块
├── # 传感器驱动
├── bno08x/                 # BNO08X九轴IMU驱动
├── jy901s/                 # JY901S九轴IMU驱动
├── hwt101/                 # HWT101陀螺仪驱动
├── mpu6050_dmp/            # MPU6050完整驱动（含DMP）
├── grayscale/              # 8通道灰度传感器
├── encoder/                # 编码器驱动库
│
├── # 电机驱动
├── motor/                  # 电机驱动库
├── emm_v5/                 # Emm_V5步进电机驱动
├── tb6612/                 # TB6612FNG双路DC电机驱动
├── a4950/                  # A4950双路DC电机驱动
│
├── # 其他外设
├── ebtn/                   # 按键驱动库
├── ad9833/                 # AD9833 DDS驱动
├── spi_flash/              # GD25Qxx SPI Flash驱动
├── oled/                   # SSD1306 OLED驱动
├── waveform_gen/           # 波形发生器
├── maixcam/                # MaixCam视觉传感器协议
│
├── # 📦 第三方开源库
├── lfs/                    # LittleFS文件系统
├── u8g2/                   # U8g2图形库（37MB！）
├── WouoUI/                 # WouoUI菜单框架
│
├── # 🔧 应用层模块
├── shell/                  # LittleFS Shell
├── waveform_analyzer/      # 波形分析器
├── pid_tuner/              # PID调参工具
│
├── # 📦 Python工具库
├── simple_uart/            # Python UART管理库
├── perspective_transform/  # 透视变换工具
│
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

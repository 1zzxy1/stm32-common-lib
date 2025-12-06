# 通用代码库

> **⚠️ 个人自用项目** - 本仓库为个人学习和开发过程中整理的通用代码库，支持多平台（嵌入式MCU / Linux / PC / Python等），代码质量和文档仅供参考，不保证生产环境可用性。

万能工具库，什么都往里塞！涵盖嵌入式开发、Linux应用、网络编程、算法工具等常用功能。

## 关于本库

### 📚 项目定位

**万能工具库 - 什么都往里塞！**

这是我个人整理的仓库 , 整理了我大一大二所用过的代码库 , 学长遗留的部分资料 , 和从网友那拿的资料 , 纯个人用.

### 🎯 适用人群

- 嵌入式开发者（STM32/STC单片机）
- Linux应用开发者（Qt/网络/系统编程）
- 电赛/智能车参赛选手
- 快速原型开发
- 代码学习和参考

### 📊 库统计

- **代码模块**：45个（持续增加中）
- **文档资料**：4类（JavaWeb、LinuxQt、Linux网络、PCB）
- **示例代码**：JavaWeb开发示例（10+）
- **配置文件**：AI开发工具配置
- **支持平台**：
  - 🔸 STM32（HAL库）
  - 🔸 STC16（龙邱平台）
  - 🔸 Linux/PC（Qt/C）
  - 🔸 Python（上位机工具）
  - 🔸 JavaWeb（J2EE）
  - 🔸 通用（纯算法，跨平台）
- **覆盖领域**：
  - **嵌入式开发**：控制算法、传感器驱动、电机驱动、通信协议、显示驱动
  - **Linux/PC开发**：网络编程、多线程、系统编程、构建工具
  - **JavaWeb开发**：Session/Cookie、Ajax、WebSocket、数据库、文件上传
  - **硬件设计**：PCB原理图、电路设计参考
  - **工具配置**：AI辅助开发工具（Claude Code、MCP）
  - **学习资料**：PDF教材、技术笔记、实验指导

### ⚠️ 免责声明

- 本库代码来源于个人学习和开发积累
- 部分代码来自网友分享和开源社区
- 代码质量参差不齐，不保证生产环境可用性
- 使用本库代码造成的任何问题，作者不承担责任
- 如有侵权请联系删除

### 📮 联系方式

- GitHub Issue：https://github.com/1zzxy1/mcu-lib/issues
- 项目地址：https://github.com/1zzxy1/mcu-lib

## 库列表

### ✅ 通用算法模块

| 库名 | 说明 | 平台 | 依赖 | 来源 |
|------|------|------|------|------|
| [pid](./pid) | PID控制器，支持位置式和增量式算法 | 通用 | 无 | 忘了哪来的了 |
| [ringbuffer](./ringbuffer) | 环形缓冲区，适用于串口等数据收发 | 通用 | 无 | 从RT-Thread抄的 |
| [scheduler](./scheduler) | 任务调度器，基于时间片的非抢占式调度 | 通用 | 无 | 从RTOS抄的 |
| [kalman](./kalman) | 卡尔曼滤波器，一维信号滤波 | 通用 | 无 | 忘了哪来的了 |
| [bit_array](./bit_array) | 位数组操作库，Header-only | 通用 | 无 | 忘了哪来的了 |
| [usart_pack](./usart_pack) | 串口数据包协议，支持多类型打包/解包 | 通用 | 无 | 忘了哪来的了 |
| [fft](./fft) | FFT频谱分析，支持THD/SINAD测量 | STM32 | CMSIS-DSP | 电赛时用过 |
| [imu_fusion](./imu_fusion) | IMU九轴融合算法（Madgwick+Kalman） | 通用 | wp_math(可选) | |
| [wp_math](./wp_math) | 高性能数学库，100+优化函数（3~10倍提速） | 通用 | 无 | |
| [multi_timer](./multi_timer) | 软件定时器管理器（无需RTOS） | 通用 | 无 | 网友那拿的 |
| [lq_balance](./lq_balance) | 平衡车控制算法（双闭环PID） | STC16 | LQ系列 | 网友那拿的 |
| [math_lib](./math_lib) | 数学工具函数（map/Clamp） | 通用 | 无 | 网友那拿的 |
| [ano_dt](./ano_dt) | 匿名地面站通信协议 | 通用 | 串口 | 学长圣遗物 |

### ⚙️ 硬件驱动模块（需修改配置）

#### 传感器驱动

| 库名 | 说明 | 平台 | 依赖 | 配置要求 | 来源 |
|------|------|------|------|----------|------|
| [bno08x](./bno08x) | BNO08X九轴IMU驱动（高精度±0.5°） | STM32 | STM32 HAL | **必须修改UART/I2C接口** | 网友那拿的 |
| [jy901s](./jy901s) | JY901S九轴IMU驱动（UART直出角度） | STM32 | STM32 HAL | **必须修改UART接口** | 网友那拿的 |
| [hwt101](./hwt101) | HWT101陀螺仪驱动（带校准） | STM32 | STM32 HAL | **必须修改UART接口** | 网友那拿的 |
| [mpu6050_dmp](./mpu6050_dmp) | MPU6050完整驱动（含DMP固件） | STM32 | STM32 HAL | **必须修改I2C接口** | 网友那拿的 |
| [lq_icm20689](./lq_icm20689) | ICM20689六轴IMU驱动（SPI） | STC16 | LQ_SPI | **必须修改SPI接口** | 网友那拿的 |
| [grayscale](./grayscale) | 8通道灰度传感器驱动（I2C） | STM32 | STM32 HAL | **必须修改I2C接口** | 网友那拿的 |
| [encoder](./encoder) | 编码器驱动库，支持速度测量和位置累计 | STM32 | STM32 HAL | 修改定时器配置 | 网友那拿的 |

#### 电机驱动

| 库名 | 说明 | 平台 | 依赖 | 配置要求 | 来源 |
|------|------|------|------|----------|------|
| [motor](./motor) | 电机驱动库，支持H桥驱动芯片 | STM32 | STM32 HAL | 修改PWM定时器配置 | 网友那拿的 |
| [emm_v5](./emm_v5) | Emm_V5步进电机串口协议驱动 | STM32 | STM32 HAL | **必须修改UART接口** | 网友那拿的 |
| [tb6612](./tb6612) | TB6612FNG双路DC电机驱动 | STM32 | STM32 HAL | **必须修改PWM和GPIO配置** | 网友那拿的 |
| [a4950](./a4950) | A4950双路DC电机驱动 | STM32 | STM32 HAL | **必须修改PWM和GPIO配置** | 网友那拿的 |
| [lq_motorservo](./lq_motorservo) | 龙邱电机舵机驱动（PWM） | STC16 | LQ_PWM | **必须修改PWM接口** | 学长圣遗物 |

#### 其他外设

| 库名 | 说明 | 平台 | 依赖 | 配置要求 | 来源 |
|------|------|------|------|----------|------|
| [ebtn](./ebtn) | 按键驱动库，支持组合键和多击检测 | 通用 | bit_array | 无硬件依赖 | 网友那拿的 |
| [ad9833](./ad9833) | AD9833 DDS信号发生器驱动 | STM32 | STM32 HAL | 修改SPI和GPIO配置 | 网友那拿的 |
| [spi_flash](./spi_flash) | ⚠️ GD25Qxx SPI Flash驱动 | STM32 | STM32 HAL | **必须修改CS引脚和SPI句柄** | 网友那拿的 |
| [oled](./oled) | ⚠️ SSD1306 OLED显示驱动（基础） | STM32 | STM32 HAL | **必须修改I2C/SPI接口** | 网友那拿的 |
| [lq_oled096](./lq_oled096) | 龙邱OLED显示驱动（功能完整） | STC16 | LQ_I2C | **必须修改I2C/SPI接口** | 学长圣遗物 |
| [waveform_gen](./waveform_gen) | ⚠️ 波形发生器（DAC+DMA+Timer） | STM32 | STM32 HAL | **必须修改DAC/Timer/DMA句柄** | 网友那拿的 |
| [maixcam](./maixcam) | MaixCam视觉传感器串口协议解析 | STM32 | STM32 HAL | **必须修改UART接口** | 网友那拿的 |
| [lq_softi2c](./lq_softi2c) | 软件I2C通信库 | 通用 | GPIO | 修改GPIO定义 | 网友那拿的 |

### 📦 第三方开源库

| 库名 | 说明 | 平台 | 依赖 | 注意事项 | 来源 |
|------|------|------|------|----------|------|
| [lfs](./lfs) | LittleFS嵌入式文件系统 | 通用 | spi_flash | 需实现硬件读写接口 | 第三方开源 |
| [u8g2](./u8g2) | 功能强大的单色图形库 | 通用 | I2C/SPI | **字体库37MB，需裁剪** | 第三方开源 |
| [WouoUI](./WouoUI) | OLED菜单UI框架 | 通用 | u8g2, ebtn | 需要u8g2和按键驱动 | 第三方开源 |

### 🔧 应用层模块（特定场景）

| 库名 | 说明 | 平台 | 依赖 | 使用场景 | 来源 |
|------|------|------|------|----------|------|
| [shell](./shell) | 🔧 LittleFS命令行Shell | STM32 | lfs, spi_flash | 文件系统调试 | |
| [waveform_analyzer](./waveform_analyzer) | 🔧 波形分析器（FFT+谐波） | STM32 | fft, CMSIS-DSP | **需实现采样率函数** | 电赛时用过 |
| [pid_tuner](./pid_tuner) | 🔧 串口命令行PID调参工具 | STM32 | usart_pack | 实时PID参数调优 | 网友那拿的 |
| [lcd_menu](./lcd_menu) | 🔧 按键菜单系统 | STM32 | oled, ebtn | 参数调节、功能选择 | 网友那拿的 |

### 📦 Python工具库（上位机）

| 库名 | 说明 | 平台 | 依赖 | 适用场景 | 来源 |
|------|------|------|------|----------|------|
| [simple_uart](./simple_uart) | 完整的Python UART管理库 | PC | pyserial | 上位机串口通信、数据采集 | |
| [perspective_transform](./perspective_transform) | 图像透视变换工具 | PC | OpenCV, NumPy | 视觉伺服、图像校正 | |

### 🐧 Linux/PC开发

#### 网络编程

| 库名 | 说明 | 平台 | 依赖 | 适用场景 | 来源 |
|------|------|------|------|----------|------|
| [linux_tcp](./linux_tcp) | TCP服务器客户端（Qt） | Linux/PC | Qt5/Qt6 | 网络通信、物联网、远程控制 | 学长圣遗物 |

#### 系统编程

| 库名 | 说明 | 平台 | 依赖 | 适用场景 | 来源 |
|------|------|------|------|----------|------|
| [linux_thread](./linux_thread) | Qt线程编程示例 | Linux/PC | Qt5/Qt6 | 多线程开发、并发编程 | 学长圣遗物 |
| [makefile_example](./makefile_example) | Makefile使用示例 | Linux/PC | GNU Make, GCC | 项目构建、自动化编译 | 学长圣遗物 |

### 📚 文档资料（docs/）

| 目录 | 说明 | 内容 | 来源 |
|------|------|------|------|
| [docs/javaweb](./docs/javaweb) | JavaWeb开发文档 | PDF教材、开发笔记、技术文档 | 学长圣遗物 |
| [docs/linux_qt](./docs/linux_qt) | Linux与Qt开发文档 | Qt编程教材、实验指导、技术笔记 | 学长圣遗物 |
| [docs/linux_network](./docs/linux_network) | Linux网络管理文档 | 网络服务配置、系统管理教程 | 学长圣遗物 |
| [docs/pcb](./docs/pcb) | PCB相关资料 | 原理图、设计参考 | 学长圣遗物 |

### 💡 示例代码（examples/）

| 目录 | 说明 | 内容 | 来源 |
|------|------|------|------|
| [examples/javaweb](./examples/javaweb) | JavaWeb代码示例 | Session/Cookie、WebSocket、Excel操作、验证码等 | 学长圣遗物 |

### ⚙️ 配置文件（configs/）

| 目录 | 说明 | 内容 | 来源 |
|------|------|------|------|
| [configs/dev_tools](./configs/dev_tools) | 开发工具配置 | Claude Code、MCP、Serena、Spec Workflow配置 | 学长圣遗物 |

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
├── # 📚 文档资料
├── docs/
│   ├── javaweb/            # JavaWeb开发文档
│   ├── linux_qt/           # Linux与Qt开发文档
│   ├── linux_network/      # Linux网络管理文档
│   └── pcb/                # PCB相关资料
│
├── # 💡 示例代码
├── examples/
│   └── javaweb/            # JavaWeb代码示例
│
├── # ⚙️ 配置文件
├── configs/
│   └── dev_tools/          # 开发工具配置（Claude Code、MCP等）
│
├── LICENSE
└── README.md
```

## ⚠️ 重要提示

> **本仓库为个人学习和开发过程中整理的代码库，代码质量和文档仅供参考。**
>
> - 代码来源于个人项目、网友分享和开源社区
> - 质量参差不齐，未经充分测试
> - **不保证生产环境可用性**
> - 使用前请务必充分测试和验证
> - 硬件依赖模块需根据实际硬件修改配置
> - 如遇问题请自行调试，作者不承担任何责任

## 许可证

- `ringbuffer` 库基于 RT-Thread，采用 Apache-2.0 许可证
- 其他库采用 MIT 许可证

详见 [LICENSE](./LICENSE)

## 贡献

欢迎提交Issue和Pull Request！

---

**最后更新**：2025-12-07
**代码模块**：45个
**文档资料**：4类（JavaWeb、LinuxQt、Linux网络、PCB）
**示例代码**：JavaWeb开发示例10+
**配置文件**：AI开发工具配置
**代码行数**：约30000+行（不含文档）

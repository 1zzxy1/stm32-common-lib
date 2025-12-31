# AD9833 DDS信号发生器驱动

AD9833是一款低功耗可编程波形发生器芯片，支持正弦波、三角波、方波输出。

## 特性

- 支持多种波形输出：正弦波、三角波、方波
- 频率分辨率：0.1Hz (25MHz时钟)
- 频率范围：0 ~ 12.5MHz
- 相位分辨率：0.1度
- 软件模拟SPI，GPIO可配置
- 支持多设备实例

## 依赖

- STM32 HAL库 (gpio.h)

## 硬件连接

```
STM32          AD9833
SDATA -------> SDATA
SCLK  -------> SCLK
FSYNC -------> FSYNC
GND   -------> GND
3.3V  -------> VDD
              MCLK (需外接25MHz晶振)
```

## 使用方法

### 1. 定义设备实例

```c
#include "ad9833.h"

ad9833_t dds;
```

### 2. 初始化设备

```c
void DDS_Init(void)
{
    // 初始化AD9833，指定GPIO引脚
    ad9833_init(&dds,
                GPIOA, GPIO_PIN_0,   // SDATA
                GPIOA, GPIO_PIN_1,   // SCLK
                GPIOA, GPIO_PIN_2);  // FSYNC
}
```

### 3. 输出波形

```c
// 方法1: 一站式配置
ad9833_output(&dds, 1000.0, 0.0, AD9833_WAVE_SINE);  // 1kHz正弦波

// 方法2: 分步配置
ad9833_set_frequency(&dds, AD9833_FREQ_REG0, 1000.0);
ad9833_set_phase(&dds, AD9833_PHASE_REG0, 0.0);
ad9833_set_wave(&dds, AD9833_WAVE_SINE);
```

### 4. 切换波形

```c
ad9833_set_wave(&dds, AD9833_WAVE_TRIANGLE);  // 三角波
ad9833_set_wave(&dds, AD9833_WAVE_SQUARE);    // 方波
```

### 5. 频率扫描

```c
for (double f = 100; f <= 10000; f += 100) {
    ad9833_set_frequency(&dds, AD9833_FREQ_REG0, f);
    HAL_Delay(10);
}
```

### 6. 多设备使用

```c
ad9833_t dds1, dds2;

void Multi_DDS_Init(void)
{
    ad9833_init(&dds1, GPIOA, GPIO_PIN_0, GPIOA, GPIO_PIN_1, GPIOA, GPIO_PIN_2);
    ad9833_init(&dds2, GPIOB, GPIO_PIN_0, GPIOB, GPIO_PIN_1, GPIOB, GPIO_PIN_2);
}

// 独立控制
ad9833_output(&dds1, 1000.0, 0.0, AD9833_WAVE_SINE);
ad9833_output(&dds2, 2000.0, 90.0, AD9833_WAVE_SINE);  // 2kHz, 90度相位
```

## 配置选项

```c
// 修改默认主时钟频率 (在包含头文件前定义)
#define AD9833_MCLK 25000000UL  // 25MHz
#include "ad9833.h"

// 或运行时设置
ad9833_set_mclk(&dds, 20000000);  // 20MHz
```

## 波形类型

| 类型 | 宏定义 | 说明 |
|------|--------|------|
| 正弦波 | `AD9833_WAVE_SINE` | 标准正弦波 |
| 三角波 | `AD9833_WAVE_TRIANGLE` | 三角波 |
| 方波 | `AD9833_WAVE_SQUARE` | 方波 (MSB/2) |
| 方波2 | `AD9833_WAVE_SQUARE2` | 方波 (MSB) |

## API 概览

| 函数 | 说明 |
|------|------|
| `ad9833_init()` | 初始化设备 |
| `ad9833_set_mclk()` | 设置主时钟频率 |
| `ad9833_set_frequency()` | 设置输出频率 |
| `ad9833_set_phase()` | 设置相位 |
| `ad9833_set_wave()` | 设置波形类型 |
| `ad9833_output()` | 一站式配置输出 |
| `ad9833_reset()` | 复位设备 |
| `ad9833_enable()` | 使能输出 |
| `ad9833_disable()` | 禁用输出 |

## 频率计算

输出频率计算公式：
```
Fout = (MCLK × FREQREG) / 2^28
```

频率分辨率 (25MHz时钟)：
```
分辨率 = 25MHz / 2^28 ≈ 0.093 Hz
```

## 注意事项

1. 输出频率最大值为MCLK/2 (25MHz时钟时为12.5MHz)
2. 方波输出频率最大为MCLK/4
3. FSYNC信号低电平有效，传输期间保持低电平
4. 数据在SCLK下降沿采样

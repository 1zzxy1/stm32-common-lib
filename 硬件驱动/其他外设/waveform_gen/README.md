# Waveform Generator 波形发生器

> ⚙️ **硬件依赖模块** - 需要DAC+DMA+Timer配置

基于DAC的任意波形发生器，支持正弦波、方波、三角波输出，频率和幅度可调。

## 特性

- 支持多种波形：正弦波、方波、三角波
- 频率可调：取决于定时器时钟
- 幅度可调：0 ~ Vref/2
- DMA循环输出
- 零基准/中点基准可选
- ADC同步采样支持

## 硬件依赖

**必须配置**（在dac_app.h中修改）:

```c
#define DAC_RESOLUTION_BITS 12          // DAC分辨率
#define DAC_VREF_MV 3300                // 参考电压(mV)
#define WAVEFORM_SAMPLES 256            // 每周期采样点数
#define TIMER_INPUT_CLOCK_HZ 90000000   // 定时器时钟频率
#define DAC_TIMER_HANDLE htim6          // DAC定时器句柄
#define DAC_HANDLE hdac                 // DAC句柄
#define DAC_CHANNEL DAC_CHANNEL_1       // DAC通道
#define DAC_DMA_HANDLE hdma_dac_ch1     // DMA句柄
```

**外部句柄**（在dac_app.c中）:
```c
extern DAC_HandleTypeDef DAC_HANDLE;
extern TIM_HandleTypeDef DAC_TIMER_HANDLE;
extern DMA_HandleTypeDef DAC_DMA_HANDLE;
```

## STM32CubeMX配置

### DAC配置
1. 启用DAC Channel 1
2. 触发源：Timer 6 Trigger Out
3. 输出缓冲：启用

### Timer配置
1. 模式：内部时钟
2. 触发输出（TRGO）：Update Event
3. 自动重载：计算值（由代码设置）

### DMA配置
1. 方向：Memory to Peripheral
2. 模式：Circular
3. 数据宽度：Half Word

## 使用方法

### 1. 初始化

```c
#include "dac_app.h"

void DAC_Init(void)
{
    // 初始化: 1kHz, 1.5V峰值幅度
    dac_app_init(1000, 1500);
}
```

### 2. 切换波形

```c
// 设置正弦波
dac_app_set_waveform(WAVEFORM_SINE);

// 设置方波
dac_app_set_waveform(WAVEFORM_SQUARE);

// 设置三角波
dac_app_set_waveform(WAVEFORM_TRIANGLE);
```

### 3. 调整频率

```c
// 设置频率为5kHz
dac_app_set_frequency(5000);
```

### 4. 调整幅度

```c
// 设置峰值幅度为1V (1000mV)
dac_app_set_amplitude(1000);
```

### 5. 基准模式

```c
// 基于零点（0 ~ 幅度*2）
dac_app_set_zero_based(1);

// 基于中点（Vref/2 ± 幅度）
dac_app_set_zero_based(0);
```

## API概览

| 函数 | 说明 |
|------|------|
| `dac_app_init()` | 初始化波形发生器 |
| `dac_app_set_waveform()` | 设置波形类型 |
| `dac_app_set_frequency()` | 设置输出频率 |
| `dac_app_set_amplitude()` | 设置峰值幅度 |
| `dac_app_set_zero_based()` | 设置基准模式 |
| `dac_app_get_amplitude()` | 获取当前幅度 |
| `dac_app_get_zero_based()` | 获取基准模式 |
| `dac_app_get_update_frequency()` | 获取DAC更新频率 |
| `dac_app_get_adc_sampling_interval_us()` | 获取ADC采样间隔 |
| `dac_app_config_adc_sync()` | 配置ADC同步 |

## 波形类型

```c
typedef enum {
    WAVEFORM_SINE,      // 正弦波
    WAVEFORM_SQUARE,    // 方波
    WAVEFORM_TRIANGLE   // 三角波
} dac_waveform_t;
```

## 频率计算

```
DAC更新频率 = 波形频率 × 采样点数
定时器ARR = 定时器时钟 / DAC更新频率 - 1
```

示例：
- 波形频率：1kHz
- 采样点数：256
- DAC更新频率：256kHz
- 定时器时钟：90MHz
- ARR = 90000000 / 256000 - 1 = 350

## 基准模式说明

### 中点基准（默认）
- 输出范围：Vref/2 ± 幅度
- 适用场景：双极性信号（AC耦合）
- 示例：1.65V ± 1V = 0.65V ~ 2.65V

### 零点基准
- 输出范围：0 ~ 幅度×2
- 适用场景：单极性信号
- 示例：0V ~ 2V

## ADC同步功能

用于波形分析应用，同步ADC采样：

```c
// 启用同步，ADC采样频率 = DAC更新频率 × 1
dac_app_config_adc_sync(1, 1);

// ADC采样频率 = DAC更新频率 × 4
dac_app_config_adc_sync(1, 4);

// 禁用同步
dac_app_config_adc_sync(0, 1);
```

## 典型应用

### 信号源
```c
// 1kHz正弦波，1V幅度
dac_app_init(1000, 1000);
dac_app_set_waveform(WAVEFORM_SINE);
```

### 函数发生器
```c
// 频率扫描
for (uint32_t freq = 100; freq <= 10000; freq += 100) {
    dac_app_set_frequency(freq);
    HAL_Delay(100);
}
```

### 波形切换演示
```c
dac_waveform_t waveforms[] = {
    WAVEFORM_SINE,
    WAVEFORM_SQUARE,
    WAVEFORM_TRIANGLE
};

for (int i = 0; i < 3; i++) {
    dac_app_set_waveform(waveforms[i]);
    HAL_Delay(1000);
}
```

## 配置参数

| 参数 | 说明 | 典型值 |
|------|------|--------|
| `WAVEFORM_SAMPLES` | 每周期采样点 | 256 |
| `DAC_RESOLUTION_BITS` | DAC分辨率 | 12 |
| `DAC_VREF_MV` | 参考电压 | 3300 |
| `TIMER_INPUT_CLOCK_HZ` | 定时器时钟 | 90000000 |

## 频率范围

**最大频率**:
```
Fmax = TIMER_CLOCK / WAVEFORM_SAMPLES / 2
     = 90MHz / 256 / 2 ≈ 175kHz
```

**最小频率**:
```
Fmin = TIMER_CLOCK / 65536 / WAVEFORM_SAMPLES
     = 90MHz / 65536 / 256 ≈ 5Hz
```

## 注意事项

1. 修改句柄名称必须与CubeMX生成的一致
2. 定时器时钟需根据实际配置修改
3. 频率过高可能失真（采样点不足）
4. DMA必须配置为Circular模式
5. DAC输出需要外部滤波电路（可选）
6. 输出幅度受DAC分辨率限制

## 性能指标

- DAC分辨率：12位（4096级）
- 电压精度：Vref / 4096 ≈ 0.8mV
- 频率精度：取决于定时器分频精度
- 波形失真：<1%（正弦波，256点）

## 内存占用

- 代码：~6KB
- RAM：512字节（波形缓冲区，256×2字节）
- Stack：<100字节

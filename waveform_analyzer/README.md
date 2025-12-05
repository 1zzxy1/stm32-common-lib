# Waveform Analyzer 波形分析器

> 🔬 **应用层模块** - 需要FFT库和项目特定配置

基于FFT的波形分析工具，支持波形类型识别、频率测量、谐波分析、相位测量等功能。

## 特性

- 波形类型识别：DC、正弦波、方波、三角波
- 精确频率测量（抛物线插值）
- Vpp/RMS/均值测量
- 3次/5次谐波分析
- 相位测量（FFT法/过零点法）
- 相位差计算
- 频率映射（支持变采样率）

## 强依赖

**必须**配合以下组件：
- ✅ **CMSIS-DSP** - ARM FFT库
- ✅ **fft模块** - FFT工具函数
- ✅ **项目特定函数** - ADC采样函数

**需要实现的函数**:
```c
// 获取ADC采样间隔(微秒)
float dac_app_get_adc_sampling_interval_us(void);
```

**需要修改的头文件**:
```c
#include "mydefine.h"  // 包含项目特定定义
```

## 核心结构

### 波形类型枚举
```c
typedef enum {
    ADC_WAVEFORM_DC = 0,       // 直流信号
    ADC_WAVEFORM_SINE = 1,     // 正弦波
    ADC_WAVEFORM_SQUARE = 2,   // 方波
    ADC_WAVEFORM_TRIANGLE = 3, // 三角波
    ADC_WAVEFORM_UNKNOWN = 255 // 未知波形
} ADC_WaveformType;
```

### 谐波信息
```c
typedef struct {
    float frequency;    // 谐波频率
    float amplitude;    // 谐波幅度
    float phase;        // 谐波相位
    float relative_amp; // 相对基波幅度比
} HarmonicComponent;
```

### 完整波形信息
```c
typedef struct {
    ADC_WaveformType waveform_type;   // 波形类型
    float frequency;                  // 基波频率(Hz)
    float vpp;                        // 峰峰值(V)
    float mean;                       // 均值(V)
    float rms;                        // 有效值(V)
    float phase;                      // 相位(弧度)
    HarmonicComponent third_harmonic; // 3次谐波
    HarmonicComponent fifth_harmonic; // 5次谐波
} WaveformInfo;
```

## 使用方法

### 1. 初始化

```c
#include "waveform_analyzer_app.h"

void Analyzer_Init(void)
{
    My_FFT_Init();
}
```

### 2. 获取完整波形信息

```c
uint32_t adc_buffer[1024];  // ADC采样缓冲区

// 采集ADC数据
// ...

// 分析波形
WaveformInfo info = Get_Waveform_Info(adc_buffer);

printf("波形类型: %s\r\n", GetWaveformTypeString(info.waveform_type));
printf("频率: %.2f Hz\r\n", info.frequency);
printf("Vpp: %.3f V\r\n", info.vpp);
printf("RMS: %.3f V\r\n", info.rms);
printf("相位: %.2f 弧度\r\n", info.phase);
printf("3次谐波: %.2f Hz (%.2f%%)\r\n",
       info.third_harmonic.frequency,
       info.third_harmonic.relative_amp * 100);
```

### 3. 单独功能调用

```c
// 只测量Vpp和RMS
float mean, rms;
float vpp = Get_Waveform_Vpp(adc_buffer, &mean, &rms);

// 只测量频率
float freq = Get_Waveform_Frequency(adc_buffer);

// 只识别波形类型
ADC_WaveformType type = Get_Waveform_Type(adc_buffer);

// 测量相位
float phase = Get_Waveform_Phase(adc_buffer, freq);

// 过零点法测相位（更精确）
float phase_zc = Get_Waveform_Phase_ZeroCrossing(adc_buffer, freq);
```

### 4. 相位差测量

```c
uint32_t ch1_buffer[1024];
uint32_t ch2_buffer[1024];
float frequency = 1000.0f;  // 信号频率

// 测量两路信号相位差
float phase_diff = Get_Phase_Difference(ch1_buffer, ch2_buffer, frequency);

printf("相位差: %.2f 弧度 (%.2f 度)\r\n",
       phase_diff, phase_diff * 180.0f / PI);
```

## API概览

### 初始化
- `My_FFT_Init()` - 初始化FFT模块

### 波形参数测量
- `Get_Waveform_Vpp()` - 测量峰峰值/均值/RMS
- `Get_Waveform_Frequency()` - 测量频率
- `Get_Waveform_Type()` - 识别波形类型
- `Get_Waveform_Info()` - 获取完整信息

### 相位测量
- `Get_Waveform_Phase()` - FFT法测相位
- `Get_Waveform_Phase_ZeroCrossing()` - 过零点法测相位
- `Calculate_Phase_Difference()` - 计算相位差
- `Get_Phase_Difference()` - 测量两路相位差

### 谐波分析
- `Analyze_Harmonics()` - 分析3/5次谐波

### FFT辅助
- `Perform_FFT()` - 执行FFT变换
- `Analyze_Frequency_And_Type()` - 频率和类型综合分析
- `Get_Component_Phase()` - 获取频率分量相位

### 频率映射（项目特定）
- `Map_Input_To_FFT_Frequency()` - 输入频率映射到FFT频率
- `Map_FFT_To_Input_Frequency()` - FFT频率映射到输入频率

## 波形识别算法

基于谐波特征识别：

| 波形 | 3次谐波 | 5次谐波 | 识别阈值 |
|------|---------|---------|----------|
| **正弦波** | <5% | <5% | 纯净 |
| **方波** | ~33% | ~20% | 3次>20% |
| **三角波** | ~11% | ~4% | 3次>6.7% |
| **DC** | 0% | 0% | DC>基波×5 |

## 测量精度

### 频率测量
- 分辨率：采样率 / FFT点数
- 精度提升：抛物线插值
- 典型精度：±0.1Hz

### 幅度测量
- 分辨率：3.3V / 4096 ≈ 0.8mV
- 精度：±2%

### 相位测量
- FFT法：±5°
- 过零点法：±1°（适用于正弦波）

## 频率映射说明

此模块包含项目特定的频率映射函数，用于处理变采样率场景：

```c
// 根据输入频率范围动态调整采样率
float Map_Input_To_FFT_Frequency(float input_frequency);
float Map_FFT_To_Input_Frequency(float fft_frequency);
```

**使用场景**：当ADC采样率根据信号频率动态调整时，需要映射函数进行频率校正。

**通用项目**：如果采样率固定，可以删除映射函数或直接返回输入值。

## 典型应用

### 示波器功能
```c
WaveformInfo ch1 = Get_Waveform_Info(ch1_buffer);
WaveformInfo ch2 = Get_Waveform_Info(ch2_buffer);

printf("CH1: %s, %.2f Hz, %.3f Vpp\r\n",
       GetWaveformTypeString(ch1.waveform_type),
       ch1.frequency, ch1.vpp);
```

### 失真分析
```c
WaveformInfo info = Get_Waveform_Info(adc_buffer);

if (info.waveform_type == ADC_WAVEFORM_SINE) {
    float thd = (info.third_harmonic.relative_amp +
                 info.fifth_harmonic.relative_amp) * 100;
    printf("THD: %.2f%%\r\n", thd);
}
```

### 相位计
```c
float phase_diff = Get_Phase_Difference(ch1_buf, ch2_buf, 1000.0f);
printf("相位差: %.1f 度\r\n", phase_diff * 180.0f / PI);
```

## 配置要求

### FFT参数
```c
#define FFT_LENGTH 1024  // FFT点数（必须与fft模块一致）
```

### ADC要求
- 采样缓冲区：uint32_t[FFT_LENGTH]
- 分辨率：12位
- 参考电压：3.3V
- 采样率：根据信号频率确定

### 采样率建议
```
采样率 ≥ 信号频率 × 10  （奈奎斯特定理 × 5倍余量）
```

## 注意事项

1. **必须实现**采样间隔获取函数
2. **FFT点数**必须与fft模块一致
3. **ADC缓冲区**必须填满FFT_LENGTH个点
4. **频率映射**函数需要根据项目调整或删除
5. 波形识别精度受噪声影响
6. 谐波分析仅支持3次和5次
7. 相位测量对正弦波效果最佳

## 移植步骤

1. 实现`dac_app_get_adc_sampling_interval_us()`
2. 修改或删除频率映射函数
3. 调整ADC电压换算公式（如需要）
4. 配置FFT_LENGTH与实际ADC缓冲区一致

## 内存占用

- 代码：~10KB
- RAM：FFT缓冲区（1024×8字节 = 8KB）
- Stack：~500字节

## 扩展功能

可添加：
- 更多次谐波分析
- THD计算
- SNR测量
- 频谱显示
- 波形录制

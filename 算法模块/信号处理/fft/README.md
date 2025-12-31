# FFT 频谱分析模块

基于ARM CMSIS-DSP库的FFT频谱分析模块，支持幅度谱计算、峰值频率插值、THD/SINAD测量等功能。

## 特性

- 基于ARM CMSIS-DSP库，计算高效
- 支持Hanning窗函数，减少频谱泄漏
- 幅度谱自动归一化
- 抛物线插值精确定位峰值频率
- THD (总谐波失真) 计算，含泄漏补偿
- THD+N (总谐波失真加噪声) 计算
- SINAD (信纳比) 计算
- 可配置FFT点数

## 依赖

- ARM CMSIS-DSP库 (arm_math.h)
- 数学库 (math.h)

## 使用方法

### 1. 配置FFT点数

在包含头文件前定义FFT点数（可选，默认1024）：

```c
#define FFT_LENGTH 1024  // 必须是4的幂次方: 64, 256, 1024, 4096
#include "fft.h"
```

### 2. 初始化

```c
void System_Init(void)
{
    fft_init();  // 初始化FFT模块
}
```

### 3. 计算频谱

```c
float adc_samples[1024];  // ADC采样数据
float sampling_freq = 100000.0f;  // 采样频率100kHz

// 执行FFT计算
fft_calculate_spectrum(adc_samples, 1024);

// 获取精确峰值频率
float peak_freq_raw = fft_get_peak_frequency(sampling_freq);
float peak_freq = fft_round_to_nearest_k(peak_freq_raw);  // 四舍五入到1kHz

// 计算THD
float thd = fft_calculate_thd(peak_freq, sampling_freq);
printf("Peak: %.0f Hz, THD: %.3f%%\n", peak_freq, thd);
```

### 4. 访问频谱数据

```c
// 遍历频谱数据
float freq_resolution = fft_get_resolution(sampling_freq);

for (uint16_t i = 1; i < FFT_LENGTH / 2; i++)
{
    float freq = fft_bin_to_freq(i, sampling_freq);
    float magnitude = fft_magnitude[i];
    printf("%.1f Hz: %.6f V\n", freq, magnitude);
}
```

### 5. 信号质量分析

```c
float fundamental_freq = 1000.0f;  // 基波频率
float sampling_freq = 100000.0f;

// 计算各项指标
float thd = fft_calculate_thd(fundamental_freq, sampling_freq);
float thd_n = fft_calculate_thd_n(fundamental_freq, sampling_freq);
float sinad = fft_calculate_sinad(fundamental_freq, sampling_freq);

printf("THD: %.3f%%\n", thd);
printf("THD+N: %.3f%%\n", thd_n);
printf("SINAD: %.1f dB\n", sinad);
```

## API 概览

| 函数 | 说明 |
|------|------|
| `fft_init()` | 初始化FFT模块 |
| `fft_generate_hanning_window()` | 生成Hanning窗 |
| `fft_calculate_spectrum()` | 计算FFT频谱 |
| `fft_get_peak_frequency()` | 获取精确峰值频率 |
| `fft_round_to_nearest_k()` | 频率四舍五入到1kHz |
| `fft_calculate_thd()` | 计算THD |
| `fft_calculate_thd_n()` | 计算THD+N |
| `fft_calculate_sinad()` | 计算SINAD |
| `fft_get_resolution()` | 获取频率分辨率 |
| `fft_bin_to_freq()` | 频点索引转频率 |
| `fft_freq_to_bin()` | 频率转频点索引 |

## 全局缓冲区

| 变量 | 说明 |
|------|------|
| `fft_input_buffer[]` | FFT输入复数缓冲区 |
| `fft_magnitude[]` | 幅度谱数组 |
| `fft_window_buffer[]` | 窗函数系数 |

## 技术说明

### 频率分辨率

```
分辨率 = 采样频率 / FFT点数
```

例如：100kHz采样，1024点FFT → 分辨率约97.7Hz

### 幅度谱归一化

- 直流分量：`FFT结果 / N * 窗函数校正`
- 交流分量：`FFT结果 * 2 / N * 窗函数校正`

乘以2是因为单边谱需要补偿负频率能量。

### THD计算

```
THD = sqrt(∑谐波功率) / sqrt(基波功率) × 100%
```

- 计算2-15次谐波
- 包含频谱泄漏补偿
- 噪声底抑制

### SINAD计算

```
SINAD(dB) = -20 × log10(THD+N / 100)
```

## 注意事项

1. FFT点数必须是4的幂次方（Radix-4算法要求）
2. 输入数据长度不足时自动零填充
3. 幅度谱仅前半部分有效（奈奎斯特定理）
4. 需要足够的RAM存储缓冲区（约12KB @1024点）

## 内存占用

| FFT点数 | 输入缓冲区 | 幅度谱 | 窗函数 | 总计 |
|---------|-----------|--------|--------|------|
| 256 | 2KB | 1KB | 1KB | 4KB |
| 1024 | 8KB | 4KB | 4KB | 16KB |
| 4096 | 32KB | 16KB | 16KB | 64KB |

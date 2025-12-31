/**
 ******************************************************************************
 * @file    fft.c
 * @brief   FFT频谱分析模块实现
 * @version 1.0.0
 ******************************************************************************
 */

#include "fft.h"
#include <string.h>
#include <math.h>

/* FFT实例 */
static arm_cfft_radix4_instance_f32 fft_instance;

/* 全局缓冲区 */
float fft_input_buffer[FFT_LENGTH * 2];
float fft_magnitude[FFT_LENGTH];
float fft_window_buffer[FFT_LENGTH];

/**
 * @brief 生成Hanning窗函数系数
 */
void fft_generate_hanning_window(void)
{
    for (uint16_t i = 0; i < FFT_LENGTH; i++)
    {
        /* Hanning窗公式: w(n) = 0.5 * (1 - cos(2*pi*n / (N-1))) */
        fft_window_buffer[i] = 0.5f * (1.0f - arm_cos_f32(2.0f * 3.14159265f * i / (FFT_LENGTH - 1)));
    }
}

/**
 * @brief FFT模块初始化
 */
void fft_init(void)
{
    /* 初始化radix-4 FFT实例 */
    arm_cfft_radix4_init_f32(&fft_instance, FFT_LENGTH, 0, 1);
    /* 参数: FFT点数, ifftFlag(0=正向FFT), bitReverseFlag(1=正常顺序输出) */

    /* 生成Hanning窗函数 */
    fft_generate_hanning_window();
}

/**
 * @brief 计算FFT频谱
 */
void fft_calculate_spectrum(float *input_data, uint16_t data_length)
{
    uint16_t actual_length = (data_length > FFT_LENGTH) ? FFT_LENGTH : data_length;

    /* 1. 清空缓冲区 */
    memset(fft_input_buffer, 0, sizeof(fft_input_buffer));
    memset(fft_magnitude, 0, sizeof(fft_magnitude));

    /* 2. 数据预处理：转换为复数格式 (可选应用窗函数) */
    for (uint16_t i = 0; i < actual_length; i++)
    {
        fft_input_buffer[2 * i] = input_data[i];      /* 实部 */
        fft_input_buffer[2 * i + 1] = 0.0f;           /* 虚部 */
    }
    /* 不足FFT_LENGTH的部分已清零，相当于零填充 */

    /* 3. 执行FFT计算 */
    arm_cfft_radix4_f32(&fft_instance, fft_input_buffer);

    /* 4. 计算幅度谱 */
    arm_cmplx_mag_f32(fft_input_buffer, fft_magnitude, FFT_LENGTH);

    /* 5. 幅度谱归一化 */
    /* Hanning窗能量校正系数 (窗函数平均损失50%能量) */
    float window_correction = 2.0f;

    for (uint16_t i = 0; i < FFT_LENGTH / 2; i++)
    {
        if (i == 0)
        {
            /* 直流分量: 幅度 = FFT结果 / N */
            fft_magnitude[i] = fft_magnitude[i] / FFT_LENGTH * window_correction;
        }
        else
        {
            /* 交流分量: 幅度 = FFT结果 * 2 / N (单边谱) */
            fft_magnitude[i] = fft_magnitude[i] * 2.0f / FFT_LENGTH * window_correction;
        }
    }
}

/**
 * @brief 使用抛物线插值获取精确峰值频率
 */
float fft_get_peak_frequency(float sampling_freq)
{
    float freq_resolution = sampling_freq / FFT_LENGTH;

    /* 1. 寻找最大幅度点 */
    float max_magnitude = 0.0f;
    uint16_t max_index = 1;
    for (uint16_t i = 1; i < FFT_LENGTH / 2; i++)
    {
        if (fft_magnitude[i] > max_magnitude)
        {
            max_magnitude = fft_magnitude[i];
            max_index = i;
        }
    }

    /* 边界检查 */
    if (max_index <= 1 || max_index >= (FFT_LENGTH / 2 - 1))
    {
        return max_index * freq_resolution;
    }

    /* 2. 三点抛物线插值 */
    float y1 = fft_magnitude[max_index - 1];
    float y2 = fft_magnitude[max_index];
    float y3 = fft_magnitude[max_index + 1];

    /* 计算抛物线顶点偏移量 */
    float denominator = y1 - 2.0f * y2 + y3;
    float delta = 0.0f;

    if (fabsf(denominator) > 1e-10f)
    {
        delta = 0.5f * (y1 - y3) / denominator;
    }

    /* 限制偏移量范围 */
    if (delta > 0.5f) delta = 0.5f;
    if (delta < -0.5f) delta = -0.5f;

    return (max_index + delta) * freq_resolution;
}

/**
 * @brief 将频率四舍五入到最近的1kHz
 */
float fft_round_to_nearest_k(float frequency)
{
    return roundf(frequency / 1000.0f) * 1000.0f;
}

/**
 * @brief 计算总谐波失真 (THD)
 */
float fft_calculate_thd(float fundamental_freq, float sampling_freq)
{
    float freq_resolution = sampling_freq / FFT_LENGTH;
    float fundamental_power = 0.0f;
    float harmonic_power = 0.0f;

    /* 动态计算噪声底 */
    float noise_floor = 0.0f;
    for (uint16_t i = FFT_LENGTH / 4; i < FFT_LENGTH / 2; i++)
    {
        noise_floor += fft_magnitude[i] * fft_magnitude[i];
    }
    noise_floor = sqrtf(noise_floor / (FFT_LENGTH / 4)) * 2.0f;

    /* 1. 找到基波频点 */
    uint16_t fundamental_bin = (uint16_t)(fundamental_freq / freq_resolution + 0.5f);

    if (fundamental_bin < 1 || fundamental_bin >= FFT_LENGTH / 2)
    {
        return 0.0f;
    }

    /* 2. 计算基波功率 (含泄漏补偿) */
    float leakage_weight = 0.3f;
    fundamental_power = fft_magnitude[fundamental_bin] * fft_magnitude[fundamental_bin];

    if (fundamental_bin > 1)
    {
        fundamental_power += leakage_weight * fft_magnitude[fundamental_bin - 1] * fft_magnitude[fundamental_bin - 1];
    }
    if (fundamental_bin < FFT_LENGTH / 2 - 1)
    {
        fundamental_power += leakage_weight * fft_magnitude[fundamental_bin + 1] * fft_magnitude[fundamental_bin + 1];
    }

    /* 3. 累加谐波功率 (2-15次谐波) */
    for (uint8_t harmonic = 2; harmonic <= 15; harmonic++)
    {
        uint16_t harmonic_bin = harmonic * fundamental_bin;

        if (harmonic_bin >= FFT_LENGTH / 2)
        {
            break;
        }

        float harmonic_magnitude = fft_magnitude[harmonic_bin];

        /* 噪声底抑制 */
        if (harmonic_magnitude > noise_floor)
        {
            float harmonic_mag_sq = harmonic_magnitude * harmonic_magnitude;

            /* 谐波泄漏补偿 */
            float h_leak_weight = leakage_weight * 0.5f;
            if (harmonic_bin > 1 && fft_magnitude[harmonic_bin - 1] > noise_floor)
            {
                harmonic_mag_sq += h_leak_weight * fft_magnitude[harmonic_bin - 1] * fft_magnitude[harmonic_bin - 1];
            }
            if (harmonic_bin < FFT_LENGTH / 2 - 1 && fft_magnitude[harmonic_bin + 1] > noise_floor)
            {
                harmonic_mag_sq += h_leak_weight * fft_magnitude[harmonic_bin + 1] * fft_magnitude[harmonic_bin + 1];
            }
            harmonic_power += harmonic_mag_sq;
        }
    }

    /* 4. 计算THD */
    if (fundamental_power > noise_floor * noise_floor)
    {
        float thd = sqrtf(harmonic_power) / sqrtf(fundamental_power) * 100.0f;
        return (thd > 100.0f) ? 100.0f : thd;
    }

    return 0.0f;
}

/**
 * @brief 计算THD+N (总谐波失真加噪声)
 */
float fft_calculate_thd_n(float fundamental_freq, float sampling_freq)
{
    float freq_resolution = sampling_freq / FFT_LENGTH;
    float fundamental_power = 0.0f;
    float total_power = 0.0f;

    uint16_t fundamental_bin = (uint16_t)(fundamental_freq / freq_resolution + 0.5f);

    if (fundamental_bin < 1 || fundamental_bin >= FFT_LENGTH / 2)
    {
        return 0.0f;
    }

    /* 计算基波功率 */
    float leakage_weight = 0.3f;
    fundamental_power = fft_magnitude[fundamental_bin] * fft_magnitude[fundamental_bin];

    if (fundamental_bin > 1)
    {
        fundamental_power += leakage_weight * fft_magnitude[fundamental_bin - 1] * fft_magnitude[fundamental_bin - 1];
    }
    if (fundamental_bin < FFT_LENGTH / 2 - 1)
    {
        fundamental_power += leakage_weight * fft_magnitude[fundamental_bin + 1] * fft_magnitude[fundamental_bin + 1];
    }

    /* 计算总功率 (排除直流) */
    for (uint16_t i = 1; i < FFT_LENGTH / 2; i++)
    {
        total_power += fft_magnitude[i] * fft_magnitude[i];
    }

    /* 计算THD+N */
    float distortion_noise = total_power - fundamental_power;
    if (fundamental_power > 0.0f && distortion_noise >= 0.0f)
    {
        float thd_n = sqrtf(distortion_noise) / sqrtf(fundamental_power) * 100.0f;
        return (thd_n > 200.0f) ? 200.0f : thd_n;
    }

    return 0.0f;
}

/**
 * @brief 计算SINAD (信纳比)
 */
float fft_calculate_sinad(float fundamental_freq, float sampling_freq)
{
    float thd_n = fft_calculate_thd_n(fundamental_freq, sampling_freq);

    if (thd_n > 0.0f)
    {
        /* SINAD(dB) = -20*log10(THD+N/100) */
        return -20.0f * log10f(thd_n / 100.0f);
    }

    return 0.0f;
}

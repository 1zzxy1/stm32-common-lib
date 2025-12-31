/**
 ******************************************************************************
 * @file    fft.h
 * @brief   FFT频谱分析模块 - 基于ARM CMSIS-DSP库
 * @version 1.0.0
 ******************************************************************************
 * @attention
 *
 * 基于ARM CMSIS-DSP库的FFT频谱分析模块
 * 支持Hanning窗、幅度谱计算、峰值频率插值、THD计算等功能
 *
 * 依赖: ARM CMSIS-DSP库 (arm_math.h)
 *
 ******************************************************************************
 */

#ifndef _FFT_H_
#define _FFT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "arm_math.h"

/* 配置选项 */
#ifndef FFT_LENGTH
#define FFT_LENGTH 1024  /* FFT点数，必须是4的幂次方 (64, 256, 1024, 4096) */
#endif

/* 全局缓冲区声明 */
extern float fft_input_buffer[FFT_LENGTH * 2];  /* FFT输入复数缓冲区 (实部、虚部交错) */
extern float fft_magnitude[FFT_LENGTH];         /* FFT幅度谱 */
extern float fft_window_buffer[FFT_LENGTH];     /* 窗函数系数 */

/**
 * @brief FFT模块初始化
 * @note  初始化FFT实例并生成Hanning窗
 */
void fft_init(void);

/**
 * @brief 生成Hanning窗函数系数
 * @note  用于减少频谱泄漏
 */
void fft_generate_hanning_window(void);

/**
 * @brief 计算FFT频谱
 * @param input_data: 输入采样数据
 * @param data_length: 数据长度 (最大FFT_LENGTH)
 * @note  计算结果存储在fft_magnitude数组中
 */
void fft_calculate_spectrum(float *input_data, uint16_t data_length);

/**
 * @brief 使用抛物线插值获取精确峰值频率
 * @param sampling_freq: 采样频率 (Hz)
 * @retval 精确的峰值频率
 */
float fft_get_peak_frequency(float sampling_freq);

/**
 * @brief 将频率四舍五入到最近的1kHz
 * @param frequency: 原始频率
 * @retval 四舍五入后的频率
 */
float fft_round_to_nearest_k(float frequency);

/**
 * @brief 计算总谐波失真 (THD)
 * @param fundamental_freq: 基波频率 (Hz)
 * @param sampling_freq: 采样频率 (Hz)
 * @retval THD值 (百分比)
 */
float fft_calculate_thd(float fundamental_freq, float sampling_freq);

/**
 * @brief 计算THD+N (总谐波失真加噪声)
 * @param fundamental_freq: 基波频率 (Hz)
 * @param sampling_freq: 采样频率 (Hz)
 * @retval THD+N值 (百分比)
 */
float fft_calculate_thd_n(float fundamental_freq, float sampling_freq);

/**
 * @brief 计算SINAD (信纳比)
 * @param fundamental_freq: 基波频率 (Hz)
 * @param sampling_freq: 采样频率 (Hz)
 * @retval SINAD值 (dB)
 */
float fft_calculate_sinad(float fundamental_freq, float sampling_freq);

/**
 * @brief 获取频率分辨率
 * @param sampling_freq: 采样频率 (Hz)
 * @retval 频率分辨率 (Hz)
 */
static inline float fft_get_resolution(float sampling_freq)
{
    return sampling_freq / FFT_LENGTH;
}

/**
 * @brief 获取指定频点的频率值
 * @param bin_index: 频点索引
 * @param sampling_freq: 采样频率 (Hz)
 * @retval 对应的频率 (Hz)
 */
static inline float fft_bin_to_freq(uint16_t bin_index, float sampling_freq)
{
    return bin_index * sampling_freq / FFT_LENGTH;
}

/**
 * @brief 获取指定频率对应的频点索引
 * @param freq: 频率 (Hz)
 * @param sampling_freq: 采样频率 (Hz)
 * @retval 频点索引
 */
static inline uint16_t fft_freq_to_bin(float freq, float sampling_freq)
{
    return (uint16_t)(freq * FFT_LENGTH / sampling_freq + 0.5f);
}

#ifdef __cplusplus
}
#endif

#endif /* _FFT_H_ */

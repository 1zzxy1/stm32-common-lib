#ifndef __DAC_APP_H
#define __DAC_APP_H

#include "mydefine.h"

// --- Copyright ---
// Copyright (c) 2024 MiCu Electronics Studio. All rights reserved.

// --- 配置 (请根据您的硬件和 CubeMX 配置修改) ---
#define DAC_RESOLUTION_BITS 12                         // DAC 分辨率 (位数)
#define DAC_MAX_VALUE ((1 << DAC_RESOLUTION_BITS) - 1) // DAC 最大数字值 (例如 12位 -> 4095)
#define DAC_VREF_MV 3300                               // DAC 参考电压 (毫伏)
#define WAVEFORM_SAMPLES 256                           // 每个波形周期的采样点数 (影响内存和最高频率)
#define TIMER_INPUT_CLOCK_HZ 90000000                  // 输入到 DAC 定时器的时钟频率 (Hz), 请根据实际配置修改!
#define DAC_TIMER_HANDLE htim6                         // 用于触发 DAC 的 TIM 句柄 (来自 CubeMX)
#define DAC_HANDLE hdac                                // DAC 句柄 (来自 CubeMX)
#define DAC_CHANNEL DAC_CHANNEL_1                      // 使用的 DAC 通道 (来自 CubeMX)
// 注意: 下面的 DMA 句柄名需要根据 CubeMX 中为 DAC 通道配置的 DMA 流确定
#define DAC_DMA_HANDLE hdma_dac_ch1 // DAC DMA 通道句柄 (来自 CubeMX)

// --- ADC采样同步配置 ---
#define ADC_DAC_SYNC_ENABLE 1       // 是否启用ADC与DAC同步 (0:禁用, 1:启用)
#define ADC_SYNC_TIMER_HANDLE htim3 // 用于触发ADC的定时器句柄
#define ADC_SAMPLING_MULTIPLIER 1   // ADC采样频率相对于DAC更新频率的倍数

// --- 波形类型枚举 ---
typedef enum
{
    WAVEFORM_SINE,    // 正弦波
    WAVEFORM_SQUARE,  // 方波
    WAVEFORM_TRIANGLE // 三角波
} dac_waveform_t;

// --- 函数原型 ---
/**
 * @brief 初始化 DAC 应用库
 * @param initial_freq_hz: 初始波形频率 (Hz)
 * @param initial_peak_amplitude_mv: 初始波形峰值幅度 (毫伏, 相对中心电压 Vref/2)
 * @retval None
 */
void dac_app_init(uint32_t initial_freq_hz, uint16_t initial_peak_amplitude_mv);

/**
 * @brief 设置输出波形类型
 * @param type: 波形类型 (dac_waveform_t 枚举)
 * @retval HAL_StatusTypeDef: 操作状态
 */
HAL_StatusTypeDef dac_app_set_waveform(dac_waveform_t type);

/**
 * @brief 设置输出波形频率
 * @param freq_hz: 新的频率 (Hz)
 * @retval HAL_StatusTypeDef: 操作状态
 */
HAL_StatusTypeDef dac_app_set_frequency(uint32_t freq_hz);

/**
 * @brief 设置输出波形峰值幅度
 * @param peak_amplitude_mv: 新的峰值幅度 (毫伏, 0 到 Vref/2)
 * @retval HAL_StatusTypeDef: 操作状态
 */
HAL_StatusTypeDef dac_app_set_amplitude(uint16_t peak_amplitude_mv);

/**
 * @brief 配置ADC采样同步功能
 * @param enable: 是否启用同步 (0:禁用, 1:启用)
 * @param multiplier: ADC采样频率相对于DAC更新频率的倍数
 * @retval HAL_StatusTypeDef: 操作状态
 */
HAL_StatusTypeDef dac_app_config_adc_sync(uint8_t enable, uint8_t multiplier);

/**
 * @brief 获取当前DAC更新频率，用于外部ADC同步配置
 * @retval uint32_t: DAC每秒更新点数 (Hz * WAVEFORM_SAMPLES)
 */
uint32_t dac_app_get_update_frequency(void);

/**
 * @brief 获取当前波形峰值幅度
 * @retval uint16_t: 当前峰值幅度(毫伏)
 */
uint16_t dac_app_get_amplitude(void);

float dac_app_get_adc_sampling_interval_us(void);

/**
 * @brief 设置波形基准模式
 * @param enable: 是否基于零点(0:基于中点, 1:基于零点)
 * @retval HAL_StatusTypeDef: 操作状态
 */
HAL_StatusTypeDef dac_app_set_zero_based(uint8_t enable);

/**
 * @brief 获取当前波形基准模式
 * @retval uint8_t: 当前模式(0:基于中点, 1:基于零点)
 */
uint8_t dac_app_get_zero_based(void);
#endif // __DAC_APP_H

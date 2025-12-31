#ifndef __Waveform_Analyzer_APP_H
#define __Waveform_Analyzer_APP_H

#include "stdio.h"
#include "string.h"
#include "stdarg.h"
#include "stdint.h"
#include "stdlib.h"

#include "WouoUI.h"    
#include "WouoUI_user.h" 
#include "u8g2.h"
#include "i2c.h"
#include "main.h"
#include "usart.h"
#include "math.h"
#include "adc.h"
#include "tim.h"
#include "dac.h"
#include "oled.h"
#include "lfs.h"
#include "lfs_port.h"
#include "gd25qxx.h"
#include "scheduler.h"
#include "ringbuffer.h"
#include "arm_math.h"

#include "oled_app.h"
#include "adc_app.h"
#include "dac_app.h"
#include "led_app.h"
#include "btn_app.h"
#include "flash_app.h"
#include "usart_app.h"
#include "shell_app.h"

// 波形类型定义（使用枚举类型更清晰）
typedef enum
{
    ADC_WAVEFORM_DC = 0,       // 直流信号
    ADC_WAVEFORM_SINE = 1,     // 正弦波
    ADC_WAVEFORM_SQUARE = 2,   // 方波
    ADC_WAVEFORM_TRIANGLE = 3, // 三角波
    ADC_WAVEFORM_UNKNOWN = 255 // 未知波形
} ADC_WaveformType;

// 谐波分量信息结构体
typedef struct
{
    float frequency;    // 分量频率
    float amplitude;    // 分量幅度
    float phase;        // 分量相位
    float relative_amp; // 相对于基波的幅度比
} HarmonicComponent;

// 扩展的波形信息结构体（包含谐波分量信息）
typedef struct
{
    ADC_WaveformType waveform_type;   // 波形类型枚举
    float frequency;                  // 基波频率，单位Hz
    float vpp;                        // 峰峰值，单位V
    float mean;                       // 均值，单位V
    float rms;                        // 有效值，单位V
    float phase;                      // 基波相位，单位弧度
    HarmonicComponent third_harmonic; // 三次谐波信息
    HarmonicComponent fifth_harmonic; // 五次谐波信息
} WaveformInfo;

// 函数声明
void My_FFT_Init(void);

// 频率映射函数
float Map_Input_To_FFT_Frequency(float input_frequency);
float Map_FFT_To_Input_Frequency(float fft_frequency);

// 基本波形分析函数
float Get_Waveform_Vpp(uint32_t *adc_val_buffer_f, float *mean, float *rms);
float Get_Waveform_Frequency(uint32_t *adc_val_buffer_f);
ADC_WaveformType Get_Waveform_Type(uint32_t *adc_val_buffer_f);

// FFT相关函数
void Perform_FFT(uint32_t *adc_val_buffer_f);
ADC_WaveformType Analyze_Frequency_And_Type(uint32_t *adc_val_buffer_f, float *signal_frequency);

// 相位计算函数
float Get_Waveform_Phase(uint32_t *adc_val_buffer_f, float frequency);
float Get_Waveform_Phase_ZeroCrossing(uint32_t *adc_val_buffer_f, float frequency);
float Calculate_Phase_Difference(float phase1, float phase2);
float Get_Phase_Difference(uint32_t *adc_val_buffer1, uint32_t *adc_val_buffer2, float frequency);

// 谐波分析函数
void Analyze_Harmonics(uint32_t *adc_val_buffer_f, WaveformInfo *waveform_info);
float Get_Component_Phase(float *fft_buffer, int component_idx);

// 辅助函数
const char *GetWaveformTypeString(ADC_WaveformType waveform);

// 综合接口
WaveformInfo Get_Waveform_Info(uint32_t *adc_val_buffer_f);

#endif // Waveform_Analyzer_APP_H

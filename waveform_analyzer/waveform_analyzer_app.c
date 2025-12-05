#include "waveform_analyzer_app.h"
#include <float.h>

#define FFT_LENGTH 1024 // 这个和adc采样点数需要保持一致
arm_cfft_radix4_instance_f32 scfft;
float FFT_InputBuf[FFT_LENGTH * 2];
float FFT_OutputBuf[FFT_LENGTH];

/**
 * @brief 输入频率到FFT频率的映射函数
 * @param input_frequency 输入频率(Hz)
 * @return 映射后的FFT频率(Hz)
 */
float Map_Input_To_FFT_Frequency(float input_frequency)
{
    // 基于规律的跳变点和乘数映射
    if (input_frequency <= 2600.0f)
    {
        return input_frequency; // 比例为1.0
    }
    else if (input_frequency <= 6100.0f)
    {
        return input_frequency * 2.0f; // 比例为2.0
    }
    else if (input_frequency <= 8100.0f)
    {
        return input_frequency * 3.0f; // 比例为3.0
    }
    else if (input_frequency <= 11100.0f)
    {
        return input_frequency * 4.0f; // 比例为4.0
    }
    else if (input_frequency <= 14100.0f)
    {
        return input_frequency * 5.0f; // 比例为5.0
    }
    else if (input_frequency <= 17100.0f)
    {
        return input_frequency * 6.0f; // 比例为6.0
    }
    else if (input_frequency <= 19600.0f)
    {
        return input_frequency * 7.0f; // 比例为7.0
    }
    else if (input_frequency <= 21600.0f)
    {
        return input_frequency * 8.0f; // 比例为8.0
    }
    else if (input_frequency <= 25100.0f)
    {
        return input_frequency * 9.0f; // 比例为9.0
    }
    else if (input_frequency <= 26600.0f)
    {
        return input_frequency * 10.0f; // 比例为10.0
    }
    else if (input_frequency <= 29600.0f)
    {
        return input_frequency * 11.0f; // 比例为11.0
    }
    else if (input_frequency <= 32100.0f)
    {
        return input_frequency * 12.0f; // 比例为12.0
    }
    else
    {
        return input_frequency * 13.0f; // 比例为13.0或更高
    }
}

/**
 * @brief 从FFT频率反推输入频率
 * @param fft_frequency FFT频率(Hz)
 * @return 估计的输入频率(Hz)
 */
float Map_FFT_To_Input_Frequency(float fft_frequency)
{
    // 定义跳变点
    const float breaks[] = {2600.0f, 6100.0f, 8100.0f, 11100.0f, 14100.0f,
                            17100.0f, 19600.0f, 21600.0f, 25100.0f, 26600.0f,
                            29600.0f, 32100.0f};
    // 对应的除数
    const float dividers[] = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f,
                              9.0f, 10.0f, 11.0f, 12.0f, 13.0f};

    // 计算最大跳变点乘以其对应的比例
    float last_break_fft = breaks[sizeof(breaks) / sizeof(breaks[0]) - 1] * dividers[sizeof(breaks) / sizeof(breaks[0])];

    // 如果FFT频率大于最大跳变点对应的FFT频率
    if (fft_frequency > last_break_fft)
    {
        return fft_frequency / dividers[sizeof(dividers) / sizeof(dividers[0]) - 1];
    }

    // 通过二分查找确定频率范围
    for (int i = 0; i < sizeof(breaks) / sizeof(breaks[0]); i++)
    {
        float break_fft = breaks[i] * dividers[i];
        float next_break_fft = (i < sizeof(breaks) / sizeof(breaks[0]) - 1) ? breaks[i + 1] * dividers[i + 1] : FLT_MAX;

        if (fft_frequency <= break_fft)
        {
            return fft_frequency / dividers[i];
        }
        else if (fft_frequency < next_break_fft)
        {
            return fft_frequency / dividers[i + 1];
        }
    }

    // 默认情况
    return fft_frequency / 13.0f;
}

/**
 * @brief 初始化FFT模块
 */
void My_FFT_Init(void)
{
    arm_cfft_radix4_init_f32(&scfft, FFT_LENGTH, 0, 1); // 初始化scfft结构体,设定FFT参数
}

/**
 * @brief 计算波形的峰峰值、均值和有效值
 * @param adc_val_buffer_f ADC采样缓冲区
 * @param mean 指向存储均值的变量的指针
 * @param rms 指向存储有效值的变量的指针
 * @return 峰峰值（伏特）
 */
float Get_Waveform_Vpp(uint32_t *adc_val_buffer_f, float *mean, float *rms)
{
    // 初始化为极值
    float min_val = 3.3f; // 初始化为最大可能电压
    float max_val = 0.0f; // 初始化为最小可能电压

    float sum = 0.0f;         // 用于计算均值
    float sum_squares = 0.0f; // 用于计算有效值
    float voltage;

    // 遍历所有采样点找出最大值和最小值，并计算均值和有效值
    for (int i = 0; i < FFT_LENGTH; i++)
    {
        voltage = (float)adc_val_buffer_f[i] / 4096.0f * 3.3f;

        // 峰峰值计算 - 更新最大和最小值
        if (voltage > max_val)
            max_val = voltage;
        if (voltage < min_val)
            min_val = voltage;

        // 均值计算 - 累加所有电压值
        sum += voltage;

        // 有效值计算 - 累加所有电压值的平方
        sum_squares += voltage * voltage;
    }

    // 计算均值
    *mean = sum / (float)FFT_LENGTH;

    // 计算有效值（RMS）
    *rms = sqrtf(sum_squares / (float)FFT_LENGTH);

    // 计算并返回峰峰值
    return max_val - min_val;
}

/**
 * @brief 执行FFT变换并填充FFT输出缓冲区
 * @param adc_val_buffer_f ADC采样缓冲区
 */
void Perform_FFT(uint32_t *adc_val_buffer_f)
{
    // 将ADC值转换为浮点数据并填充FFT输入缓冲区
    for (int i = 0; i < FFT_LENGTH; i++)
    {
        FFT_InputBuf[2 * i] = (float)adc_val_buffer_f[i] / 4096.0f * 3.3f; // 实部
        FFT_InputBuf[2 * i + 1] = 0;                                       // 虚部
    }

    // 执行FFT计算
    arm_cfft_radix4_f32(&scfft, FFT_InputBuf);                  // FFT计算（基4）
    arm_cmplx_mag_f32(FFT_InputBuf, FFT_OutputBuf, FFT_LENGTH); // 取模得幅值
}

/**
 * @brief 获取FFT结果中特定频率分量的相位
 * @param fft_buffer FFT计算结果缓冲区
 * @param component_idx 分量索引
 * @return 相位角（弧度制，范围为-PI到PI）
 */
float Get_Component_Phase(float *fft_buffer, int component_idx)
{
    // 从FFT结果中获取实部和虚部
    float real_part = fft_buffer[2 * component_idx];
    float imag_part = fft_buffer[2 * component_idx + 1];

    // 计算相位角（弧度制）
    float phase = atan2f(imag_part, real_part);

    return phase;
}

/**
 * @brief 计算信号的相位角
 * @param adc_val_buffer_f ADC采样缓冲区
 * @param frequency 信号频率
 * @return 相位角（弧度制，范围为-PI到PI）
 */
float Get_Waveform_Phase(uint32_t *adc_val_buffer_f, float frequency)
{
    // 如果是DC信号或未知信号，相位无意义
    if (frequency <= 0.0f)
    {
        return 0.0f;
    }

    // 执行FFT变换
    for (int i = 0; i < FFT_LENGTH; i++)
    {
        FFT_InputBuf[2 * i] = (float)adc_val_buffer_f[i] / 4096.0f * 3.3f; // 实部
        FFT_InputBuf[2 * i + 1] = 0;                                       // 虚部
    }

    arm_cfft_radix4_f32(&scfft, FFT_InputBuf); // FFT计算

    // 将输入频率映射到FFT频率
    float fft_frequency = Map_Input_To_FFT_Frequency(frequency);

    // 计算对应于基频的FFT输出索引
    float sampling_interval_us = dac_app_get_adc_sampling_interval_us();
    float sampling_frequency = 1000000.0f / sampling_interval_us;
    int fundamental_idx = (int)(fft_frequency * FFT_LENGTH / sampling_frequency + 0.5f);

    // 限制索引在有效范围内
    if (fundamental_idx <= 0 || fundamental_idx >= FFT_LENGTH / 2)
    {
        return 0.0f;
    }

    return Get_Component_Phase(FFT_InputBuf, fundamental_idx);
}

/**
 * @brief 使用过零点检测计算信号相位
 * @param adc_val_buffer_f ADC采样缓冲区
 * @param frequency 信号频率
 * @return 相位角（弧度制，范围为0到2PI）
 */
float Get_Waveform_Phase_ZeroCrossing(uint32_t *adc_val_buffer_f, float frequency)
{
    // 如果是DC信号或未知信号，相位无意义
    if (frequency <= 0.0f)
    {
        return 0.0f;
    }

    float sampling_interval_us = dac_app_get_adc_sampling_interval_us();
    float sampling_frequency = 1000000.0f / sampling_interval_us;

    // 计算平均值
    float mean = 0.0f;
    for (int i = 0; i < FFT_LENGTH; i++)
    {
        mean += (float)adc_val_buffer_f[i] / 4096.0f * 3.3f;
    }
    mean /= (float)FFT_LENGTH;

    // 寻找第一个过零点（从负到正）
    int zero_crossing_idx = -1;
    for (int i = 1; i < FFT_LENGTH; i++)
    {
        float current_val = (float)adc_val_buffer_f[i] / 4096.0f * 3.3f - mean;
        float prev_val = (float)adc_val_buffer_f[i - 1] / 4096.0f * 3.3f - mean;

        if (prev_val < 0.0f && current_val >= 0.0f)
        {
            // 找到过零点
            zero_crossing_idx = i;
            break;
        }
    }

    // 如果没有找到过零点
    if (zero_crossing_idx < 0)
    {
        return 0.0f;
    }

    // 精确过零点位置（线性插值）
    float prev_val = (float)adc_val_buffer_f[zero_crossing_idx - 1] / 4096.0f * 3.3f - mean;
    float current_val = (float)adc_val_buffer_f[zero_crossing_idx] / 4096.0f * 3.3f - mean;

    // 计算过零点的精确位置
    float fraction = -prev_val / (current_val - prev_val);
    float exact_crossing = (float)(zero_crossing_idx - 1) + fraction;

    // 获取输入频率对应的FFT频率
    float fft_frequency = Map_Input_To_FFT_Frequency(frequency);

    // 计算周期（采样点数）
    float samples_per_period = sampling_frequency / fft_frequency;

    // 计算相位角（0到2PI）
    float phase = 2.0f * PI * exact_crossing / samples_per_period;

    // 将相位限制在0到2PI范围内
    while (phase < 0.0f)
        phase += 2.0f * PI;
    while (phase >= 2.0f * PI)
        phase -= 2.0f * PI;

    return phase;
}

/**
 * @brief 计算两个信号之间的相位差
 * @param phase1 第一个信号的相位
 * @param phase2 第二个信号的相位
 * @return 相位差（弧度制，范围为-PI到PI）
 */
float Calculate_Phase_Difference(float phase1, float phase2)
{
    float phase_diff = phase1 - phase2;

    // 将相位差限制在-PI到PI范围内
    while (phase_diff > PI)
        phase_diff -= 2.0f * PI;
    while (phase_diff <= -PI)
        phase_diff += 2.0f * PI;

    return phase_diff;
}

/**
 * @brief 获取波形的频率
 * @param adc_val_buffer_f ADC采样缓冲区
 * @return 频率（Hz）
 */
float Get_Waveform_Frequency(uint32_t *adc_val_buffer_f)
{
    float sampling_interval_us = dac_app_get_adc_sampling_interval_us(); // 获取采样间隔（微秒）
    float sampling_frequency = 1000000.0f / sampling_interval_us;        // 计算采样频率（Hz）

    // 执行FFT变换
    Perform_FFT(adc_val_buffer_f);

    // 查找基频（忽略直流分量）
    float max_harmonic = 0;   // 最大谐波分量
    int max_harmonic_idx = 0; // 最大谐波分量索引

    for (int i = 1; i < FFT_LENGTH / 2; i++) // 只需考察前一半频谱（Nyquist频率）
    {
        if (FFT_OutputBuf[i] > max_harmonic)
        {
            max_harmonic = FFT_OutputBuf[i];
            max_harmonic_idx = i;
        }
    }

    // 计算FFT频率（Hz）
    // 频率计算公式: freq = idx * sampling_freq / FFT_LENGTH
    float fft_frequency = (float)max_harmonic_idx * sampling_frequency / (float)FFT_LENGTH;

    // 检查直流信号
    if (FFT_OutputBuf[0] > max_harmonic * 5.0f)
    {
        return 0.0f; // 如果是直流信号，返回0Hz
    }

    // 将FFT频率映射回输入频率
    return Map_FFT_To_Input_Frequency(fft_frequency);
}

/**
 * @brief 分析波形的谐波成分
 * @param adc_val_buffer_f ADC采样缓冲区
 * @param waveform_info 指向波形信息结构体的指针
 */
void Analyze_Harmonics(uint32_t *adc_val_buffer_f, WaveformInfo *waveform_info)
{
    float sampling_interval_us = dac_app_get_adc_sampling_interval_us();
    float sampling_frequency = 1000000.0f / sampling_interval_us;

    // 确保有有效的基频
    if (waveform_info->frequency <= 0.0f)
    {
        // 初始化谐波信息为0
        waveform_info->third_harmonic.frequency = 0.0f;
        waveform_info->third_harmonic.amplitude = 0.0f;
        waveform_info->third_harmonic.phase = 0.0f;
        waveform_info->third_harmonic.relative_amp = 0.0f;

        waveform_info->fifth_harmonic.frequency = 0.0f;
        waveform_info->fifth_harmonic.amplitude = 0.0f;
        waveform_info->fifth_harmonic.phase = 0.0f;
        waveform_info->fifth_harmonic.relative_amp = 0.0f;
        return;
    }

    // 执行FFT变换
    for (int i = 0; i < FFT_LENGTH; i++)
    {
        FFT_InputBuf[2 * i] = (float)adc_val_buffer_f[i] / 4096.0f * 3.3f; // 实部
        FFT_InputBuf[2 * i + 1] = 0;                                       // 虚部
    }

    arm_cfft_radix4_f32(&scfft, FFT_InputBuf); // FFT计算

    // 计算幅度谱
    float magnitude_spectrum[FFT_LENGTH];
    arm_cmplx_mag_f32(FFT_InputBuf, magnitude_spectrum, FFT_LENGTH);

    // 将输入频率映射到FFT频率
    float fft_frequency = Map_Input_To_FFT_Frequency(waveform_info->frequency);

    // 计算基频对应的FFT索引
    int fundamental_idx = (int)(fft_frequency * FFT_LENGTH / sampling_frequency + 0.5f);

    // 获取基波幅度和相位
    float fundamental_amp = magnitude_spectrum[fundamental_idx];
    float fundamental_phase = Get_Component_Phase(FFT_InputBuf, fundamental_idx);

    // 计算三次谐波索引和范围
    int expected_third_harmonic_idx = 3 * fundamental_idx;
    int third_search_start = expected_third_harmonic_idx - fundamental_idx / 4;
    int third_search_end = expected_third_harmonic_idx + fundamental_idx / 4;

    // 确保索引在有效范围内
    if (third_search_start < fundamental_idx)
        third_search_start = fundamental_idx + 1;
    if (third_search_end >= FFT_LENGTH / 2)
        third_search_end = FFT_LENGTH / 2 - 1;

    // 寻找三次谐波峰值
    float third_harmonic_amp = 0.0f;
    int third_harmonic_idx = 0;

    for (int i = third_search_start; i <= third_search_end; i++)
    {
        if (magnitude_spectrum[i] > third_harmonic_amp)
        {
            third_harmonic_amp = magnitude_spectrum[i];
            third_harmonic_idx = i;
        }
    }

    // 计算五次谐波索引和范围
    int expected_fifth_harmonic_idx = 5 * fundamental_idx;
    int fifth_search_start = expected_fifth_harmonic_idx - fundamental_idx / 4;
    int fifth_search_end = expected_fifth_harmonic_idx + fundamental_idx / 4;

    // 确保索引在有效范围内
    if (fifth_search_start < third_harmonic_idx + 1)
        fifth_search_start = third_harmonic_idx + 1;
    if (fifth_search_end >= FFT_LENGTH / 2)
        fifth_search_end = FFT_LENGTH / 2 - 1;

    // 寻找五次谐波峰值
    float fifth_harmonic_amp = 0.0f;
    int fifth_harmonic_idx = 0;

    for (int i = fifth_search_start; i <= fifth_search_end; i++)
    {
        if (magnitude_spectrum[i] > fifth_harmonic_amp)
        {
            fifth_harmonic_amp = magnitude_spectrum[i];
            fifth_harmonic_idx = i;
        }
    }

    // 忽略幅度太小的谐波（小于基波幅度的5%）
    if (third_harmonic_amp < fundamental_amp * 0.05f)
    {
        third_harmonic_amp = 0.0f;
        third_harmonic_idx = 0;
    }

    if (fifth_harmonic_amp < fundamental_amp * 0.05f)
    {
        fifth_harmonic_amp = 0.0f;
        fifth_harmonic_idx = 0;
    }

    // 计算谐波的相位
    float third_harmonic_phase = (third_harmonic_idx > 0) ? Get_Component_Phase(FFT_InputBuf, third_harmonic_idx) : 0.0f;

    float fifth_harmonic_phase = (fifth_harmonic_idx > 0) ? Get_Component_Phase(FFT_InputBuf, fifth_harmonic_idx) : 0.0f;

    // 计算谐波FFT频率
    float third_harmonic_fft_freq = (third_harmonic_idx > 0) ? (float)third_harmonic_idx * sampling_frequency / (float)FFT_LENGTH : 0.0f;
    float fifth_harmonic_fft_freq = (fifth_harmonic_idx > 0) ? (float)fifth_harmonic_idx * sampling_frequency / (float)FFT_LENGTH : 0.0f;
    
    // 将谐波FFT频率映射回输入频率
    float third_harmonic_freq = (third_harmonic_idx > 0) ? Map_FFT_To_Input_Frequency(third_harmonic_fft_freq) : 0.0f;
    float fifth_harmonic_freq = (fifth_harmonic_idx > 0) ? Map_FFT_To_Input_Frequency(fifth_harmonic_fft_freq) : 0.0f;

    // 填充三次谐波信息
    waveform_info->third_harmonic.frequency = third_harmonic_freq;
    waveform_info->third_harmonic.amplitude = third_harmonic_amp;
    waveform_info->third_harmonic.phase = third_harmonic_phase;
    waveform_info->third_harmonic.relative_amp = (fundamental_amp > 0.0f) ? third_harmonic_amp / fundamental_amp : 0.0f;

    // 填充五次谐波信息
    waveform_info->fifth_harmonic.frequency = fifth_harmonic_freq;
    waveform_info->fifth_harmonic.amplitude = fifth_harmonic_amp;
    waveform_info->fifth_harmonic.phase = fifth_harmonic_phase;
    waveform_info->fifth_harmonic.relative_amp = (fundamental_amp > 0.0f) ? fifth_harmonic_amp / fundamental_amp : 0.0f;

    // 计算相对于基波的相位差
    if (third_harmonic_idx > 0)
    {
        float third_phase_diff = Calculate_Phase_Difference(third_harmonic_phase, fundamental_phase);
        waveform_info->third_harmonic.phase = third_phase_diff; // 存储相位差而不是绝对相位
    }

    if (fifth_harmonic_idx > 0)
    {
        float fifth_phase_diff = Calculate_Phase_Difference(fifth_harmonic_phase, fundamental_phase);
        waveform_info->fifth_harmonic.phase = fifth_phase_diff; // 存储相位差而不是绝对相位
    }

    // // 打印谐波分析结果
    // my_printf(&huart1, "基波: 频率=%.2f Hz, 幅度=%.4f, 相位=%.2f rad\r\n",
    //           waveform_info->frequency, fundamental_amp, fundamental_phase);

    // my_printf(&huart1, "三次谐波: 频率=%.2f Hz, 幅度=%.4f (%.2f%% of 基波), 相位差=%.2f rad\r\n",
    //           waveform_info->third_harmonic.frequency,
    //           waveform_info->third_harmonic.amplitude,
    //           waveform_info->third_harmonic.relative_amp * 100.0f,
    //           waveform_info->third_harmonic.phase);

    // my_printf(&huart1, "五次谐波: 频率=%.2f Hz, 幅度=%.4f (%.2f%% of 基波), 相位差=%.2f rad\r\n",
    //           waveform_info->fifth_harmonic.frequency,
    //           waveform_info->fifth_harmonic.amplitude,
    //           waveform_info->fifth_harmonic.relative_amp * 100.0f,
    //           waveform_info->fifth_harmonic.phase);
}

/**
 * @brief 执行FFT变换并分析频谱特征
 * @param adc_val_buffer_f ADC采样缓冲区
 * @param signal_frequency 指向存储频率的变量的指针
 * @return 波形类型枚举值
 */
ADC_WaveformType Analyze_Frequency_And_Type(uint32_t *adc_val_buffer_f, float *signal_frequency)
{
    float sampling_interval_us = dac_app_get_adc_sampling_interval_us();
    float sampling_frequency = 1000000.0f / sampling_interval_us;

    // 执行FFT变换
    for (int i = 0; i < FFT_LENGTH; i++)
    {
        FFT_InputBuf[2 * i] = (float)adc_val_buffer_f[i] / 4096.0f * 3.3f; // 实部
        FFT_InputBuf[2 * i + 1] = 0;                                       // 虚部
    }

    arm_cfft_radix4_f32(&scfft, FFT_InputBuf);                  // FFT计算
    arm_cmplx_mag_f32(FFT_InputBuf, FFT_OutputBuf, FFT_LENGTH); // 取模得幅值

    // 直流分量
    float dc_component = FFT_OutputBuf[0];

    // 1. 寻找基频（最大峰值）
    float fundamental_amp = 0.0f;
    int fundamental_idx = 0;

    for (int i = 1; i < FFT_LENGTH / 2; i++)
    {
        if (FFT_OutputBuf[i] > fundamental_amp)
        {
            fundamental_amp = FFT_OutputBuf[i];
            fundamental_idx = i;
        }
    }

    // 计算FFT频率
    float fft_frequency = (float)fundamental_idx * sampling_frequency / (float)FFT_LENGTH;
    
    // 将FFT频率映射回输入频率
    *signal_frequency = Map_FFT_To_Input_Frequency(fft_frequency);

    // 检查直流信号
    if (dc_component > fundamental_amp * 5.0f)
    {
        *signal_frequency = 0.0f;
        return ADC_WAVEFORM_DC;
    }

    // 信号强度检查
    if (fundamental_amp < 5.0f)
    {
        return ADC_WAVEFORM_UNKNOWN; // 信号太弱
    }

    // 2. 寻找三次谐波（在2*fundamental_idx到4*fundamental_idx之间）
    float third_harmonic_amp = 0.0f;
    int third_harmonic_idx = 0;

    // 定义搜索范围
    int start_third = 2 * fundamental_idx;
    int end_third = 4 * fundamental_idx;

    // 限制搜索范围在有效FFT区间内
    if (end_third > FFT_LENGTH / 2)
        end_third = FFT_LENGTH / 2;

    // 在三次谐波预期区域寻找峰值
    for (int i = start_third; i < end_third; i++)
    {
        if (FFT_OutputBuf[i] > third_harmonic_amp)
        {
            third_harmonic_amp = FFT_OutputBuf[i];
            third_harmonic_idx = i;
        }
    }

    // 3. 寻找五次谐波（在4*fundamental_idx到6*fundamental_idx之间）
    float fifth_harmonic_amp = 0.0f;
    int fifth_harmonic_idx = 0;

    // 定义搜索范围
    int start_fifth = 4 * fundamental_idx;
    int end_fifth = 6 * fundamental_idx;

    // 限制搜索范围在有效FFT区间内
    if (end_fifth > FFT_LENGTH / 2)
        end_fifth = FFT_LENGTH / 2;

    // 在五次谐波预期区域寻找峰值
    for (int i = start_fifth; i < end_fifth; i++)
    {
        if (FFT_OutputBuf[i] > fifth_harmonic_amp)
        {
            fifth_harmonic_amp = FFT_OutputBuf[i];
            fifth_harmonic_idx = i;
        }
    }

    // 检查找到的谐波是否有意义（幅度至少为基频的5%）
    if (third_harmonic_amp < fundamental_amp * 0.05f)
    {
        third_harmonic_amp = 0.0f;
    }

    if (fifth_harmonic_amp < fundamental_amp * 0.05f)
    {
        fifth_harmonic_amp = 0.0f;
    }

    // 计算谐波比例
    float third_ratio = (third_harmonic_amp > 0) ? (third_harmonic_amp / fundamental_amp) : 0;
    float fifth_ratio = (fifth_harmonic_amp > 0) ? (fifth_harmonic_amp / fundamental_amp) : 0;

    //    调试输出
//    my_printf(&huart1, "基频索引: %d, 幅度: %.3f\r\n", fundamental_idx, fundamental_amp);
//    my_printf(&huart1, "三次谐波索引: %d, 幅度: %.3f, 比例: %.3f\r\n",
//              third_harmonic_idx, third_harmonic_amp, third_ratio);
//    my_printf(&huart1, "五次谐波索引: %d, 幅度: %.3f, 比例: %.3f\r\n",
//              fifth_harmonic_idx, fifth_harmonic_amp, fifth_ratio);

    // 4. 波形判断

    // 如果没有明显谐波，是正弦波
    if (third_ratio < 0.05f && fifth_ratio < 0.05f)
    {
        return ADC_WAVEFORM_SINE;
    }

    // 理论比例
    const float TRIANGLE_THIRD_END = 1.0f / 15.0f; // 三角波三次谐波理论比例结束(因为不一定完全重合)
    const float SQUARE_THIRD_END = 1.0f / 5.0f;    // 方波三次谐波理论比例结束(因为不一定完全重合)

    if (third_ratio > SQUARE_THIRD_END)
    {
        return ADC_WAVEFORM_SQUARE; // 方波
    }
    else if (third_ratio > TRIANGLE_THIRD_END)
    {
        return ADC_WAVEFORM_TRIANGLE; // 三角波
    }
    else
    {
        return ADC_WAVEFORM_UNKNOWN; // 未知波形
    }
}

/**
 * @brief 获取波形类型
 * @param adc_val_buffer_f ADC采样缓冲区
 * @return 波形类型枚举值
 */
ADC_WaveformType Get_Waveform_Type(uint32_t *adc_val_buffer_f)
{
    float frequency;
    return Analyze_Frequency_And_Type(adc_val_buffer_f, &frequency);
}

const char *GetWaveformTypeString(ADC_WaveformType waveform)
{
    switch (waveform)
    {
    case ADC_WAVEFORM_DC:
        return "直流";
    case ADC_WAVEFORM_SINE:
        return "正弦波";
    case ADC_WAVEFORM_TRIANGLE:
        return "三角波";
    case ADC_WAVEFORM_SQUARE:
        return "方波";
    case ADC_WAVEFORM_UNKNOWN:
        return "未知波形";
    default:
        return "无效波形";
    }
}

/**
 * @brief 计算两个波形之间的相位差
 * @param adc_val_buffer1 第一个ADC采样缓冲区
 * @param adc_val_buffer2 第二个ADC采样缓冲区
 * @param frequency 信号频率
 * @return 相位差（弧度制，范围为-PI到PI）
 */
float Get_Phase_Difference(uint32_t *adc_val_buffer1, uint32_t *adc_val_buffer2, float frequency)
{
    // 如果频率无效，返回0
    if (frequency <= 0.0f)
    {
        return 0.0f;
    }

    // 计算第一个信号的相位
    float phase1 = Get_Waveform_Phase(adc_val_buffer1, frequency);

    // 计算第二个信号的相位
    float phase2 = Get_Waveform_Phase(adc_val_buffer2, frequency);

    // 计算相位差
    float phase_diff = Calculate_Phase_Difference(phase1, phase2);

    // 相位差转换为度数并打印日志
    float phase_diff_degrees = phase_diff * 180.0f / PI;

    my_printf(&huart1, "相位1: %.2f 弧度, 相位2: %.2f 弧度, 差值: %.2f 弧度 (%.2f 度)\r\n",
              phase1, phase2, phase_diff, phase_diff_degrees);

    return phase_diff;
}

/**
 * @brief 获取波形信息，包括波形类型、频率、峰峰值、均值、有效值、相位和谐波信息
 * @param adc_val_buffer_f ADC采样缓冲区
 * @return 包含波形类型、频率、峰峰值、均值、有效值、相位和谐波信息的结构体
 */
WaveformInfo Get_Waveform_Info(uint32_t *adc_val_buffer_f)
{
    // 初始化结构体，包括谐波信息
    WaveformInfo result = {
        .waveform_type = ADC_WAVEFORM_UNKNOWN,
        .frequency = 0.0f,
        .vpp = 0.0f,
        .mean = 0.0f,
        .rms = 0.0f,
        .phase = 0.0f,
        .third_harmonic = {0.0f, 0.0f, 0.0f, 0.0f},
        .fifth_harmonic = {0.0f, 0.0f, 0.0f, 0.0f}};

    // 计算峰峰值、均值和有效值
    result.vpp = Get_Waveform_Vpp(adc_val_buffer_f, &result.mean, &result.rms);

    // 获取波形类型和频率
    result.waveform_type = Analyze_Frequency_And_Type(adc_val_buffer_f, &result.frequency);

    // 计算相位（只有当波形不是DC信号且频率有效时才计算相位）
    if (result.waveform_type != ADC_WAVEFORM_DC && result.frequency > 0.0f)
    {
        result.phase = Get_Waveform_Phase(adc_val_buffer_f, result.frequency);

        // 分析谐波成分（包括幅度和相位）
        Analyze_Harmonics(adc_val_buffer_f, &result);
    }

    // 打印波形基本信息
    //    my_printf(&huart1, "波形类型: %s\r\n", GetWaveformTypeString(result.waveform_type));
    //    my_printf(&huart1, "频率: %.2f Hz, 峰峰值: %.2f V, 平均值: %.2f V, 有效值: %.2f V, 相位: %.2f 弧度\r\n",
    //              result.frequency, result.vpp, result.mean, result.rms, result.phase);

    // // 打印谐波信息
    // if (result.waveform_type != ADC_WAVEFORM_DC && result.frequency > 0.0f)
    // {
    //     my_printf(&huart1, "三次谐波: 频率=%.2f Hz, 幅度比=%.2f%%, 相位差=%.2f 弧度\r\n",
    //               result.third_harmonic.frequency,
    //               result.third_harmonic.relative_amp * 100.0f,
    //               result.third_harmonic.phase);

    //     my_printf(&huart1, "五次谐波: 频率=%.2f Hz, 幅度比=%.2f%%, 相位差=%.2f 弧度\r\n",
    //               result.fifth_harmonic.frequency,
    //               result.fifth_harmonic.relative_amp * 100.0f,
    //               result.fifth_harmonic.phase);
    // }

    return result;
}

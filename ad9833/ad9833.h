/**
 ******************************************************************************
 * @file    ad9833.h
 * @brief   AD9833 DDS信号发生器驱动库
 * @version 1.0.0
 ******************************************************************************
 * @attention
 *
 * AD9833是一款低功耗可编程波形发生器芯片
 * 支持正弦波、三角波、方波输出
 * 基于STM32 HAL库，使用软件模拟SPI
 *
 ******************************************************************************
 */

#ifndef _AD9833_H_
#define _AD9833_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

/* 默认主时钟频率 (Hz) */
#ifndef AD9833_MCLK
#define AD9833_MCLK 25000000UL  /* 25MHz */
#endif

/* 波形类型定义 */
typedef enum {
    AD9833_WAVE_SINE     = 0x2000,  /* 正弦波 */
    AD9833_WAVE_TRIANGLE = 0x2002,  /* 三角波 */
    AD9833_WAVE_SQUARE   = 0x2028,  /* 方波 (MSB/2) */
    AD9833_WAVE_SQUARE2  = 0x2020   /* 方波 (MSB) */
} ad9833_wave_t;

/* 频率寄存器选择 */
typedef enum {
    AD9833_FREQ_REG0 = 0,
    AD9833_FREQ_REG1 = 1
} ad9833_freq_reg_t;

/* 相位寄存器选择 */
typedef enum {
    AD9833_PHASE_REG0 = 0,
    AD9833_PHASE_REG1 = 1
} ad9833_phase_reg_t;

/* AD9833硬件配置结构体 */
typedef struct {
    GPIO_TypeDef *sdata_port;   /* SDATA引脚端口 */
    uint16_t sdata_pin;         /* SDATA引脚 */
    GPIO_TypeDef *sclk_port;    /* SCLK引脚端口 */
    uint16_t sclk_pin;          /* SCLK引脚 */
    GPIO_TypeDef *fsync_port;   /* FSYNC引脚端口 */
    uint16_t fsync_pin;         /* FSYNC引脚 */
} ad9833_hw_t;

/* AD9833设备结构体 */
typedef struct {
    ad9833_hw_t hw;             /* 硬件配置 */
    uint32_t mclk;              /* 主时钟频率 (Hz) */
    double freq0;               /* 频率寄存器0的值 */
    double freq1;               /* 频率寄存器1的值 */
    uint16_t phase0;            /* 相位寄存器0的值 */
    uint16_t phase1;            /* 相位寄存器1的值 */
    ad9833_wave_t wave;         /* 当前波形 */
} ad9833_t;

/**
 * @brief 初始化AD9833设备
 * @param dev: 设备实例指针
 * @param sdata_port: SDATA端口
 * @param sdata_pin: SDATA引脚
 * @param sclk_port: SCLK端口
 * @param sclk_pin: SCLK引脚
 * @param fsync_port: FSYNC端口
 * @param fsync_pin: FSYNC引脚
 * @retval 0: 成功, -1: 参数错误
 */
int8_t ad9833_init(ad9833_t *dev,
                   GPIO_TypeDef *sdata_port, uint16_t sdata_pin,
                   GPIO_TypeDef *sclk_port, uint16_t sclk_pin,
                   GPIO_TypeDef *fsync_port, uint16_t fsync_pin);

/**
 * @brief 设置主时钟频率
 * @param dev: 设备实例指针
 * @param mclk: 主时钟频率 (Hz)
 */
void ad9833_set_mclk(ad9833_t *dev, uint32_t mclk);

/**
 * @brief 设置输出频率
 * @param dev: 设备实例指针
 * @param reg: 频率寄存器 (AD9833_FREQ_REG0 或 AD9833_FREQ_REG1)
 * @param freq: 输出频率 (Hz)
 */
void ad9833_set_frequency(ad9833_t *dev, ad9833_freq_reg_t reg, double freq);

/**
 * @brief 设置相位
 * @param dev: 设备实例指针
 * @param reg: 相位寄存器 (AD9833_PHASE_REG0 或 AD9833_PHASE_REG1)
 * @param phase_deg: 相位值 (0-360度)
 */
void ad9833_set_phase(ad9833_t *dev, ad9833_phase_reg_t reg, float phase_deg);

/**
 * @brief 设置波形类型
 * @param dev: 设备实例指针
 * @param wave: 波形类型
 */
void ad9833_set_wave(ad9833_t *dev, ad9833_wave_t wave);

/**
 * @brief 选择使用的频率和相位寄存器
 * @param dev: 设备实例指针
 * @param freq_reg: 频率寄存器
 * @param phase_reg: 相位寄存器
 */
void ad9833_select_register(ad9833_t *dev, ad9833_freq_reg_t freq_reg, ad9833_phase_reg_t phase_reg);

/**
 * @brief 一站式配置输出
 * @param dev: 设备实例指针
 * @param freq: 输出频率 (Hz)
 * @param phase_deg: 相位 (度)
 * @param wave: 波形类型
 */
void ad9833_output(ad9833_t *dev, double freq, float phase_deg, ad9833_wave_t wave);

/**
 * @brief 复位设备
 * @param dev: 设备实例指针
 */
void ad9833_reset(ad9833_t *dev);

/**
 * @brief 使能输出
 * @param dev: 设备实例指针
 */
void ad9833_enable(ad9833_t *dev);

/**
 * @brief 禁用输出 (进入睡眠模式)
 * @param dev: 设备实例指针
 */
void ad9833_disable(ad9833_t *dev);

#ifdef __cplusplus
}
#endif

#endif /* _AD9833_H_ */

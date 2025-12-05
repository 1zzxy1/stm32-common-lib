/**
 ******************************************************************************
 * @file    ad9833.c
 * @brief   AD9833 DDS信号发生器驱动库实现
 * @version 1.0.0
 ******************************************************************************
 */

#include "ad9833.h"

/* 控制寄存器位定义 */
#define AD9833_B28      (1 << 13)   /* 28位频率写入模式 */
#define AD9833_HLB      (1 << 12)   /* 高/低字节选择 */
#define AD9833_FSEL     (1 << 11)   /* 频率寄存器选择 */
#define AD9833_PSEL     (1 << 10)   /* 相位寄存器选择 */
#define AD9833_RESET    (1 << 8)    /* 复位 */
#define AD9833_SLEEP1   (1 << 7)    /* 关闭MCLK */
#define AD9833_SLEEP12  (1 << 6)    /* 关闭DAC */
#define AD9833_OPBITEN  (1 << 5)    /* MSB输出使能 */
#define AD9833_DIV2     (1 << 3)    /* MSB/2 */
#define AD9833_MODE     (1 << 1)    /* 三角波模式 */

/* 寄存器地址 */
#define AD9833_REG_FREQ0    (1 << 14)
#define AD9833_REG_FREQ1    (2 << 14)
#define AD9833_REG_PHASE0   (6 << 13)
#define AD9833_REG_PHASE1   (7 << 13)

/* 私有函数声明 */
static void ad9833_delay(void);
static void ad9833_write(ad9833_t *dev, uint16_t data);

/**
 * @brief 简单延时
 */
static void ad9833_delay(void)
{
    volatile uint16_t i;
    for (i = 0; i < 2; i++);
}

/**
 * @brief 写入16位数据到AD9833
 */
static void ad9833_write(ad9833_t *dev, uint16_t data)
{
    int i;

    /* 确保SCLK和FSYNC为高 */
    HAL_GPIO_WritePin(dev->hw.sclk_port, dev->hw.sclk_pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(dev->hw.fsync_port, dev->hw.fsync_pin, GPIO_PIN_SET);

    /* 拉低FSYNC开始传输 */
    HAL_GPIO_WritePin(dev->hw.fsync_port, dev->hw.fsync_pin, GPIO_PIN_RESET);

    /* 发送16位数据，MSB优先 */
    for (i = 0; i < 16; i++) {
        /* 设置数据位 */
        if (data & 0x8000) {
            HAL_GPIO_WritePin(dev->hw.sdata_port, dev->hw.sdata_pin, GPIO_PIN_SET);
        } else {
            HAL_GPIO_WritePin(dev->hw.sdata_port, dev->hw.sdata_pin, GPIO_PIN_RESET);
        }

        /* SCLK下降沿 */
        ad9833_delay();
        HAL_GPIO_WritePin(dev->hw.sclk_port, dev->hw.sclk_pin, GPIO_PIN_RESET);
        ad9833_delay();
        /* SCLK上升沿 */
        HAL_GPIO_WritePin(dev->hw.sclk_port, dev->hw.sclk_pin, GPIO_PIN_SET);

        /* 移位到下一位 */
        data <<= 1;
    }

    /* 拉高FSYNC结束传输 */
    HAL_GPIO_WritePin(dev->hw.fsync_port, dev->hw.fsync_pin, GPIO_PIN_SET);
}

/**
 * @brief 初始化AD9833设备
 */
int8_t ad9833_init(ad9833_t *dev,
                   GPIO_TypeDef *sdata_port, uint16_t sdata_pin,
                   GPIO_TypeDef *sclk_port, uint16_t sclk_pin,
                   GPIO_TypeDef *fsync_port, uint16_t fsync_pin)
{
    if (dev == NULL) {
        return -1;
    }

    /* 保存硬件配置 */
    dev->hw.sdata_port = sdata_port;
    dev->hw.sdata_pin = sdata_pin;
    dev->hw.sclk_port = sclk_port;
    dev->hw.sclk_pin = sclk_pin;
    dev->hw.fsync_port = fsync_port;
    dev->hw.fsync_pin = fsync_pin;

    /* 默认参数 */
    dev->mclk = AD9833_MCLK;
    dev->freq0 = 0;
    dev->freq1 = 0;
    dev->phase0 = 0;
    dev->phase1 = 0;
    dev->wave = AD9833_WAVE_SINE;

    /* 配置GPIO为输出模式 */
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Pull = GPIO_NOPULL;

    GPIO_InitStruct.Pin = sdata_pin;
    HAL_GPIO_Init(sdata_port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = sclk_pin;
    HAL_GPIO_Init(sclk_port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = fsync_pin;
    HAL_GPIO_Init(fsync_port, &GPIO_InitStruct);

    /* 设置初始电平 */
    HAL_GPIO_WritePin(dev->hw.sdata_port, dev->hw.sdata_pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(dev->hw.sclk_port, dev->hw.sclk_pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(dev->hw.fsync_port, dev->hw.fsync_pin, GPIO_PIN_SET);

    /* 复位设备 */
    ad9833_reset(dev);

    return 0;
}

/**
 * @brief 设置主时钟频率
 */
void ad9833_set_mclk(ad9833_t *dev, uint32_t mclk)
{
    if (dev != NULL) {
        dev->mclk = mclk;
    }
}

/**
 * @brief 设置输出频率
 */
void ad9833_set_frequency(ad9833_t *dev, ad9833_freq_reg_t reg, double freq)
{
    if (dev == NULL) {
        return;
    }

    /* 计算频率寄存器值 */
    /* freq_reg = (freq * 2^28) / MCLK */
    double freq_factor = 268435456.0 / (double)dev->mclk;  /* 2^28 / MCLK */
    uint32_t freq_reg_val = (uint32_t)(freq * freq_factor);

    /* 分离为高14位和低14位 */
    uint16_t freq_lsb = (freq_reg_val & 0x3FFF);
    uint16_t freq_msb = ((freq_reg_val >> 14) & 0x3FFF);

    /* 添加寄存器地址 */
    uint16_t reg_addr = (reg == AD9833_FREQ_REG0) ? AD9833_REG_FREQ0 : AD9833_REG_FREQ1;
    freq_lsb |= reg_addr;
    freq_msb |= reg_addr;

    /* 写入控制字和频率数据 */
    ad9833_write(dev, AD9833_B28 | AD9833_RESET);  /* B28=1, RESET=1 */
    ad9833_write(dev, freq_lsb);
    ad9833_write(dev, freq_msb);

    /* 保存频率值 */
    if (reg == AD9833_FREQ_REG0) {
        dev->freq0 = freq;
    } else {
        dev->freq1 = freq;
    }
}

/**
 * @brief 设置相位
 */
void ad9833_set_phase(ad9833_t *dev, ad9833_phase_reg_t reg, float phase_deg)
{
    if (dev == NULL) {
        return;
    }

    /* 将角度转换为12位值 (0-4095 对应 0-360度) */
    uint16_t phase_val = (uint16_t)((phase_deg / 360.0f) * 4096.0f) & 0x0FFF;

    /* 添加寄存器地址 */
    uint16_t reg_addr = (reg == AD9833_PHASE_REG0) ? AD9833_REG_PHASE0 : AD9833_REG_PHASE1;
    phase_val |= reg_addr;

    ad9833_write(dev, phase_val);

    /* 保存相位值 */
    if (reg == AD9833_PHASE_REG0) {
        dev->phase0 = phase_val & 0x0FFF;
    } else {
        dev->phase1 = phase_val & 0x0FFF;
    }
}

/**
 * @brief 设置波形类型
 */
void ad9833_set_wave(ad9833_t *dev, ad9833_wave_t wave)
{
    if (dev == NULL) {
        return;
    }

    dev->wave = wave;
    ad9833_write(dev, wave);
}

/**
 * @brief 选择使用的频率和相位寄存器
 */
void ad9833_select_register(ad9833_t *dev, ad9833_freq_reg_t freq_reg, ad9833_phase_reg_t phase_reg)
{
    if (dev == NULL) {
        return;
    }

    uint16_t ctrl = dev->wave;

    if (freq_reg == AD9833_FREQ_REG1) {
        ctrl |= AD9833_FSEL;
    }

    if (phase_reg == AD9833_PHASE_REG1) {
        ctrl |= AD9833_PSEL;
    }

    ad9833_write(dev, ctrl);
}

/**
 * @brief 一站式配置输出
 */
void ad9833_output(ad9833_t *dev, double freq, float phase_deg, ad9833_wave_t wave)
{
    if (dev == NULL) {
        return;
    }

    /* 复位 */
    ad9833_write(dev, AD9833_RESET);

    /* 设置频率 */
    ad9833_set_frequency(dev, AD9833_FREQ_REG0, freq);

    /* 设置相位 */
    ad9833_set_phase(dev, AD9833_PHASE_REG0, phase_deg);

    /* 设置波形并启用输出 */
    dev->wave = wave;
    ad9833_write(dev, wave);  /* 清除RESET位，开始输出 */
}

/**
 * @brief 复位设备
 */
void ad9833_reset(ad9833_t *dev)
{
    if (dev == NULL) {
        return;
    }

    ad9833_write(dev, AD9833_RESET);
}

/**
 * @brief 使能输出
 */
void ad9833_enable(ad9833_t *dev)
{
    if (dev == NULL) {
        return;
    }

    ad9833_write(dev, dev->wave);  /* 写入当前波形设置，清除RESET位 */
}

/**
 * @brief 禁用输出
 */
void ad9833_disable(ad9833_t *dev)
{
    if (dev == NULL) {
        return;
    }

    ad9833_write(dev, AD9833_SLEEP1 | AD9833_SLEEP12);  /* 关闭MCLK和DAC */
}

/**
 ******************************************************************************
 * @file    motor_driver.c
 * @brief   电机驱动库 - 支持H桥驱动芯片 (A4950/TB6612等)
 * @version 1.0.0
 ******************************************************************************
 */

#include "motor_driver.h"

/* 私有函数声明 */
static uint32_t speed_to_pwm(int8_t speed);
static int8_t motor_validate(motor_t *motor);

/**
 * @brief 创建电机实例
 */
int8_t motor_create(motor_t *motor,
                    TIM_HandleTypeDef *htim,
                    uint32_t channel,
                    GPIO_TypeDef *dir_port,
                    uint16_t dir_pin,
                    uint8_t reverse)
{
    /* 参数检查 */
    if (motor == NULL || htim == NULL || dir_port == NULL) {
        return -1;
    }

    /* 检查通道有效性 */
    if (channel != TIM_CHANNEL_1 && channel != TIM_CHANNEL_2 &&
        channel != TIM_CHANNEL_3 && channel != TIM_CHANNEL_4) {
        return -1;
    }

    /* 初始化硬件配置 */
    motor->hw.htim = htim;
    motor->hw.channel = channel;
    motor->hw.dir_port = dir_port;
    motor->hw.dir_pin = dir_pin;

    /* 初始化电机状态 */
    motor->speed = 0;
    motor->state = MOTOR_STATE_STOP;
    motor->enable = 1;
    motor->reverse = reverse;

    /* 启动PWM通道 */
    HAL_TIM_PWM_Start(motor->hw.htim, motor->hw.channel);

    /* 设置初始状态：停止 */
    HAL_GPIO_WritePin(motor->hw.dir_port, motor->hw.dir_pin, GPIO_PIN_RESET);
    __HAL_TIM_SET_COMPARE(motor->hw.htim, motor->hw.channel, 0);

    return 0;
}

/**
 * @brief 设置电机速度
 */
int8_t motor_set_speed(motor_t *motor, int8_t speed)
{
    /* 参数检查 */
    if (motor_validate(motor) != 0) {
        return -1;
    }

    /* 速度范围检查 */
    if (speed < MOTOR_SPEED_MIN || speed > MOTOR_SPEED_MAX) {
        return -1;
    }

    /* 检查电机是否使能 */
    if (!motor->enable) {
        return -1;
    }

    /* 保存速度值 */
    motor->speed = speed;

    /* 处理停止 */
    if (speed == 0) {
        HAL_GPIO_WritePin(motor->hw.dir_port, motor->hw.dir_pin, GPIO_PIN_RESET);
        __HAL_TIM_SET_COMPARE(motor->hw.htim, motor->hw.channel, 0);
        motor->state = MOTOR_STATE_STOP;
        return 0;
    }

    /* 处理电机反装 */
    int8_t actual_speed = motor->reverse ? -speed : speed;
    uint32_t pwm_value;
    uint8_t dir_level;

    if (actual_speed < 0) {
        /* 反转: DIR=1, PWM=(100+speed) */
        dir_level = 1;
        pwm_value = speed_to_pwm(100 + actual_speed);
        motor->state = MOTOR_STATE_BACKWARD;
    } else {
        /* 正转: DIR=0, PWM=speed */
        dir_level = 0;
        pwm_value = speed_to_pwm(actual_speed);
        motor->state = MOTOR_STATE_FORWARD;
    }

    /* 设置方向和PWM */
    HAL_GPIO_WritePin(motor->hw.dir_port, motor->hw.dir_pin,
                     dir_level ? GPIO_PIN_SET : GPIO_PIN_RESET);
    __HAL_TIM_SET_COMPARE(motor->hw.htim, motor->hw.channel, pwm_value);

    return 0;
}

/**
 * @brief 停止电机
 */
int8_t motor_stop(motor_t *motor)
{
    if (motor_validate(motor) != 0) {
        return -1;
    }

    HAL_GPIO_WritePin(motor->hw.dir_port, motor->hw.dir_pin, GPIO_PIN_RESET);
    __HAL_TIM_SET_COMPARE(motor->hw.htim, motor->hw.channel, 0);

    motor->speed = 0;
    motor->state = MOTOR_STATE_STOP;

    return 0;
}

/**
 * @brief 获取电机状态
 */
motor_state_t motor_get_state(motor_t *motor)
{
    if (motor_validate(motor) != 0) {
        return MOTOR_STATE_ERROR;
    }
    return motor->state;
}

/**
 * @brief 获取电机当前速度
 */
int8_t motor_get_speed(motor_t *motor)
{
    if (motor_validate(motor) != 0) {
        return 0;
    }
    return motor->speed;
}

/**
 * @brief 使能/失能电机
 */
int8_t motor_enable(motor_t *motor, uint8_t enable)
{
    if (motor_validate(motor) != 0) {
        return -1;
    }

    motor->enable = enable;

    /* 失能时停止电机 */
    if (!enable) {
        HAL_GPIO_WritePin(motor->hw.dir_port, motor->hw.dir_pin, GPIO_PIN_RESET);
        __HAL_TIM_SET_COMPARE(motor->hw.htim, motor->hw.channel, 0);
        motor->speed = 0;
        motor->state = MOTOR_STATE_STOP;
    }

    return 0;
}

/* 私有函数实现 */

/**
 * @brief 将速度值转换为PWM比较值
 */
static uint32_t speed_to_pwm(int8_t speed)
{
    uint8_t abs_speed = (speed < 0) ? (uint8_t)(-speed) : (uint8_t)speed;

    if (abs_speed == 0) {
        return 0;
    }

    /* 转换为PWM值 */
    uint32_t pwm_value = (uint32_t)abs_speed * MOTOR_PWM_PERIOD / 100;

    /* 死区补偿 */
    if (pwm_value > 0 && pwm_value < MOTOR_MIN_PWM_THRESHOLD) {
        pwm_value = MOTOR_MIN_PWM_THRESHOLD;
    }

    /* 限幅 */
    if (pwm_value > MOTOR_PWM_PERIOD) {
        pwm_value = MOTOR_PWM_PERIOD;
    }

    return pwm_value;
}

/**
 * @brief 验证电机参数有效性
 */
static int8_t motor_validate(motor_t *motor)
{
    if (motor == NULL || motor->hw.htim == NULL || motor->hw.dir_port == NULL) {
        return -1;
    }
    return 0;
}

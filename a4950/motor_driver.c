/**
 ******************************************************************************
 * @file    motor_driver.c
 * @brief   A4950 电机驱动库源文件
 * @author  XiFeng
 * @date    2025-06-20
 ******************************************************************************
 * @attention
 *
 * 本库专为A4950ELJTR-T电机驱动芯片设计
 * 支持双电机独立控制，实现A4950的快慢衰减模式
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "motor_driver.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
static uint32_t Speed_To_PWM(int8_t speed);
static int8_t Motor_ValidateParams(Motor_t *motor);

/* Exported functions --------------------------------------------------------*/


/**
 * @brief 创建电机实体
 */
int8_t Motor_Create(Motor_t *motor,
                    TIM_HandleTypeDef *htim,
                    uint32_t channel,
                    GPIO_TypeDef *dir_port,
                    uint16_t dir_pin,
                    uint8_t reverse)
{
    // 参数检查
    if (motor == NULL || htim == NULL || dir_port == NULL)
    {
        return -1;
    }

    // 检查通道有效性
    if (channel != TIM_CHANNEL_1 && channel != TIM_CHANNEL_2 &&
        channel != TIM_CHANNEL_3 && channel != TIM_CHANNEL_4)
    {
        return -1;
    }

    // 初始化硬件配置
    motor->hw.htim = htim;
    motor->hw.channel = channel;
    motor->hw.dir_port = dir_port;
    motor->hw.dir_pin = dir_pin;

    // 初始化电机状态
    motor->speed = 0;
    motor->state = MOTOR_STATE_STOP;
    motor->enable = 1;
    motor->reverse = reverse;

    // 启动对应的PWM通道
    HAL_TIM_PWM_Start(motor->hw.htim, motor->hw.channel);
    
    // 设置初始状态：停止（DIR=0, PWM=0）
    HAL_GPIO_WritePin(motor->hw.dir_port, motor->hw.dir_pin, GPIO_PIN_RESET);
    __HAL_TIM_SET_COMPARE(motor->hw.htim, motor->hw.channel, 0);

    return 0;
}

/**
 * @brief 设置电机速度
 */
int8_t Motor_SetSpeed(Motor_t *motor, int8_t speed)
{
    // 参数检查
    if (Motor_ValidateParams(motor) != 0)
    {
        return -1;
    }

    // 速度范围检查
    if (speed < MOTOR_SPEED_MIN || speed > MOTOR_SPEED_MAX)
    {
        return -1;
    }

    // 检查电机是否使能
    if (!motor->enable)
    {
        return -1;
    }

    // 保存速度值
    motor->speed = speed;

    // 处理停止
    if (speed == 0)
    {
        // A4950停止逻辑：DIR=0, PWM=0
        HAL_GPIO_WritePin(motor->hw.dir_port, motor->hw.dir_pin, GPIO_PIN_RESET);
        __HAL_TIM_SET_COMPARE(motor->hw.htim, motor->hw.channel, 0);
        motor->state = MOTOR_STATE_STOP;
        return 0;
    }

    // A4950标准控制逻辑：
    // 正数：DIR=0, PWM=速度值
    // 负数：DIR=1, PWM=100+负数值
    // reverse参数：反装电机需要将速度取反
    
    int8_t actual_speed = motor->reverse ? -speed : speed;  // 反装电机速度取反
    uint32_t pwm_value;
    uint8_t dir_level;
    
    if (actual_speed < 0) 
	{
        // 负数（反转）
        dir_level = 1;
        pwm_value = Speed_To_PWM(100 + actual_speed);  // actual_speed是负数
        motor->state = MOTOR_STATE_BACKWARD;
    } 
	else 
	{
        // 正数（正转）
        dir_level = 0;
        pwm_value = Speed_To_PWM(actual_speed);
        motor->state = MOTOR_STATE_FORWARD;
    }
    
    // 设置DIR引脚
    HAL_GPIO_WritePin(motor->hw.dir_port, motor->hw.dir_pin, 
                     dir_level ? GPIO_PIN_SET : GPIO_PIN_RESET);

    // 设置PWM占空比
    __HAL_TIM_SET_COMPARE(motor->hw.htim, motor->hw.channel, pwm_value);

    return 0;
}

/**
 * @brief 停止电机
 */
int8_t Motor_Stop(Motor_t *motor)
{
    // 参数检查
    if (Motor_ValidateParams(motor) != 0)
    {
        return -1;
    }

    // A4950停止逻辑：DIR=0, PWM=0
    HAL_GPIO_WritePin(motor->hw.dir_port, motor->hw.dir_pin, GPIO_PIN_RESET);
    __HAL_TIM_SET_COMPARE(motor->hw.htim, motor->hw.channel, 0);

    // 更新状态
    motor->speed = 0;
    motor->state = MOTOR_STATE_STOP;

    return 0;
}

/**
 * @brief 获取电机状态
 */
MotorState_t Motor_GetState(Motor_t *motor)
{
    // 参数检查
    if (Motor_ValidateParams(motor) != 0)
    {
        return MOTOR_STATE_ERROR;
    }

    return motor->state;
}

/**
 * @brief 使能/失能电机
 */
int8_t Motor_Enable(Motor_t *motor, uint8_t enable)
{
    // 参数检查
    if (Motor_ValidateParams(motor) != 0)
    {
        return -1;
    }

    motor->enable = enable;

    // 如果失能，立即停止电机
    if (!enable)
    {
        // A4950停止逻辑：DIR=0, PWM=0
        HAL_GPIO_WritePin(motor->hw.dir_port, motor->hw.dir_pin, GPIO_PIN_RESET);
        __HAL_TIM_SET_COMPARE(motor->hw.htim, motor->hw.channel, 0);
        motor->speed = 0;
        motor->state = MOTOR_STATE_STOP;
    }

    return 0;
}

/* Private functions ---------------------------------------------------------*/

/**
 * @brief 将速度值转换为PWM比较值
 * @param speed: 速度值 (-100 到 +100)
 * @retval PWM比较值 (0 到 MOTOR_PWM_PERIOD)
 */
static uint32_t Speed_To_PWM(int8_t speed)
{
    uint8_t abs_speed;

    // 获取速度绝对值
    if (speed < 0)
    {
        abs_speed = (uint8_t)(-speed);
    }
    else
    {
        abs_speed = (uint8_t)speed;
    }

    // 如果速度为0，直接返回0
    if (abs_speed == 0)
    {
        return 0;
    }

    // 转换为PWM值：速度百分比 * PWM周期
    uint32_t pwm_value = (uint32_t)abs_speed * MOTOR_PWM_PERIOD / 100;

    // PWM阈值提升逻辑：如果计算出的PWM小于阈值，提升到阈值
    if (pwm_value > 0 && pwm_value < MOTOR_MIN_PWM_THRESHOLD)
    {
        pwm_value = MOTOR_MIN_PWM_THRESHOLD;
    }

    // 确保不超过最大值
    if (pwm_value > MOTOR_PWM_PERIOD)
    {
        pwm_value = MOTOR_PWM_PERIOD;
    }

    return pwm_value;
}

/**
 * @brief 验证电机参数有效性
 * @param motor: 电机实体指针
 * @retval 0: 有效, -1: 无效
 */
static int8_t Motor_ValidateParams(Motor_t *motor)
{
    if (motor == NULL ||
        motor->hw.htim == NULL ||
        motor->hw.dir_port == NULL)
    {
        return -1;
    }

    return 0;
}


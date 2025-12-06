
/**
 ******************************************************************************
 * @file   	motor_driver.c
 * @brief   TB6612 电机驱动库源文件
 * @author  (你的名字)
 * @date    2025年07月08日
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "motor_driver_tb6612.h"

/* Private function prototypes -----------------------------------------------*/
// 内部私有函数，负责将百分比速度映射为PWM值
static uint32_t Speed_To_PWM(int16_t speed);
// 内部私有函数，用于验证电机结构体指针是否有效
static int8_t Motor_ValidateParams(Motor_t *motor);

/* Exported functions --------------------------------------------------------*/

/**
 * @brief 初始化TB6612芯片，控制其STBY引脚。
 */
void TB6612_Init(uint8_t enable)
{
    // 调用此函数前，请确保STBY引脚对应的GPIO时钟已在MX_GPIO_Init()中使能
    if (enable)
    {
        HAL_GPIO_WritePin(TB6612_STBY_PORT, TB6612_STBY_PIN, GPIO_PIN_SET);
    }
    else
    {
        HAL_GPIO_WritePin(TB6612_STBY_PORT, TB6612_STBY_PIN, GPIO_PIN_RESET);
    }
}

/**
 * @brief 创建并初始化一个电机实例。
 */
int8_t Motor_Create(Motor_t *motor,
                    TIM_HandleTypeDef *htim,
                    uint32_t channel,
                    GPIO_TypeDef *in1_port, uint16_t in1_pin,
                    GPIO_TypeDef *in2_port, uint16_t in2_pin,
                    uint8_t reverse)
{
    // 1. 参数有效性检查
    if (motor == NULL || htim == NULL || in1_port == NULL || in2_port == NULL)
    {
        return -1;
    }
    // 检查PWM通道是否合法
    if (channel != TIM_CHANNEL_1 && channel != TIM_CHANNEL_2 &&
        channel != TIM_CHANNEL_3 && channel != TIM_CHANNEL_4)
    {
        return -1;
    }

    // 2. 填充硬件配置信息到结构体
    motor->hw.htim = htim;
    motor->hw.channel = channel;
    motor->hw.in1_port = in1_port;
    motor->hw.in1_pin = in1_pin;
    motor->hw.in2_port = in2_port;
    motor->hw.in2_pin = in2_pin;

    // 3. 初始化电机逻辑状态
    motor->speed = 0;
    motor->state = MOTOR_STATE_STOP;
    motor->enable = 1; // 默认软件使能
    motor->reverse = reverse;

    // 4. 启动对应的PWM通道
    HAL_TIM_PWM_Start(motor->hw.htim, motor->hw.channel);

    // 5. 设置初始物理状态：刹车 (IN1=LOW, IN2=LOW, PWM=0)
    HAL_GPIO_WritePin(motor->hw.in1_port, motor->hw.in1_pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(motor->hw.in2_port, motor->hw.in2_pin, GPIO_PIN_RESET);
    __HAL_TIM_SET_COMPARE(motor->hw.htim, motor->hw.channel, 0);

    return 0; // 初始化成功
}

/**
 * @brief 设置电机速度。
 */
int8_t Motor_SetSpeed(Motor_t *motor, int16_t speed)
{
    // 1. 参数检查
    if (Motor_ValidateParams(motor) != 0)
        return -1;
    if (speed < MOTOR_LOGIC_SPEED_MIN || speed > MOTOR_LOGIC_SPEED_MAX)
        return -1;

    // 2. 检查电机是否被软件使能
    if (!motor->enable)
        return -1;

    // 3. 保存逻辑速度值
    motor->speed = speed;

    // 4. 如果电机是反向安装的，则将速度逻辑取反
    if (motor->reverse)
    {
        speed = -speed;
    }

    // 5. 根据速度的正负，设置TB6612的IN1/IN2引脚并计算PWM值
    if (speed == 0)
    {
        // --- 刹车模式 ---
        // TB6612逻辑：IN1和IN2同为高电平或低电平即为刹车。高电平刹车效果更强。
        HAL_GPIO_WritePin(motor->hw.in1_port, motor->hw.in1_pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(motor->hw.in2_port, motor->hw.in2_pin, GPIO_PIN_SET);
        __HAL_TIM_SET_COMPARE(motor->hw.htim, motor->hw.channel, 0);
        motor->state = MOTOR_STATE_STOP;
    }
    else if (speed > 0)
    {
        // --- 正转模式 ---
        // TB6612逻辑：IN1=HIGH, IN2=LOW
        HAL_GPIO_WritePin(motor->hw.in1_port, motor->hw.in1_pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(motor->hw.in2_port, motor->hw.in2_pin, GPIO_PIN_RESET);
        __HAL_TIM_SET_COMPARE(motor->hw.htim, motor->hw.channel, Speed_To_PWM(speed));
        motor->state = MOTOR_STATE_FORWARD;
    }
    else
    { // speed < 0
        // --- 反转模式 ---
        // TB6612逻辑：IN1=LOW, IN2=HIGH
        HAL_GPIO_WritePin(motor->hw.in1_port, motor->hw.in1_pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(motor->hw.in2_port, motor->hw.in2_pin, GPIO_PIN_SET);
        __HAL_TIM_SET_COMPARE(motor->hw.htim, motor->hw.channel, Speed_To_PWM(speed));
        motor->state = MOTOR_STATE_BACKWARD;
    }

    return 0;
}

/**
 * @brief 快速停止电机 (刹车模式)。
 */
int8_t Motor_Stop(Motor_t *motor)
{
    // 直接调用SetSpeed(0)来实现刹车
    return Motor_SetSpeed(motor, 0);
}

/**
 * @brief 获取电机当前状态。
 */
MotorState_t Motor_GetState(Motor_t *motor)
{
    if (Motor_ValidateParams(motor) != 0)
    {
        return MOTOR_STATE_ERROR;
    }
    return motor->state;
}

/**
 * @brief (软件层面)使能或失能单个电机。
 */
int8_t Motor_SetEnable(Motor_t *motor, uint8_t enable)
{
    if (Motor_ValidateParams(motor) != 0)
    {
        return -1;
    }

    motor->enable = enable;

    // 如果是失能操作，则立即停止电机
    if (!enable)
    {
        Motor_Stop(motor);
    }

    return 0;
}

/* Private functions ---------------------------------------------------------*/

/**
 * @brief  将逻辑速度映射为物理PWM值(0~ARR)。
 * @param  speed: 逻辑速度值。
 * @retval 物理PWM计数值。
 */
static uint32_t Speed_To_PWM(int16_t speed)
{
    // 1. 取速度的绝对值
    int16_t abs_speed = (speed < 0) ? -speed : speed;

    // 2. 核心映射公式：
    //    物理PWM值 = (逻辑速度 / 逻辑最大速度) * 物理最大PWM值(ARR)
    //    这里 (MOTOR_PWM_MAX_PULSE - 1) 就是定时器的ARR值
    uint32_t pwm_value = (uint32_t)abs_speed * (MOTOR_PWM_MAX_PULSE - 1) / MOTOR_LOGIC_SPEED_MAX;

    // 3. 应用最小启动阈值 (可选功能)
    //    防止在速度很低时，电机因为扭力不足而不转
#if MOTOR_MIN_PHYSICAL_PWM > 0
    if (pwm_value > 0 && pwm_value < MOTOR_MIN_PHYSICAL_PWM)
    {
        pwm_value = MOTOR_MIN_PHYSICAL_PWM;
    }
#endif

    // 4. 返回最终计算出的PWM值
    return pwm_value;
}

/**
 * @brief 验证电机结构体指针及其内部关键指针是否有效。
 */
static int8_t Motor_ValidateParams(Motor_t *motor)
{
    if (motor == NULL || motor->hw.htim == NULL)
    {
        return -1; // 无效
    }
    return 0; // 有效
}



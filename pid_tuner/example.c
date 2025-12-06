/**
 ******************************************************************************
 * @file    example.c
 * @brief   PID调参库使用示例
 * @author  XiFeng
 * @date    2025-07-14
 ******************************************************************************
 * @attention
 * 
 * 本示例展示如何在STM32F4项目中集成和使用PID调参库
 * 包含完整的回调函数实现、控制器注册和串口集成方法
 * 
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "pid_tuner.h"
#include "pid.h"  // 假设项目中已有PID库

/* Private variables ---------------------------------------------------------*/
// PID调参器实例
static PidTuner_t pid_tuner;

// 示例PID控制器实例（假设项目中已定义）
extern PID_T angle_pid;    // 角度PID控制器
extern PID_T speed_pid;    // 速度PID控制器

/* Private function prototypes -----------------------------------------------*/
// 角度PID回调函数
static int8_t angle_pid_set_callback(uint8_t id, float kp, float ki, float kd);
static int8_t angle_pid_get_callback(uint8_t id, float* kp, float* ki, float* kd);
static int8_t angle_pid_save_callback(uint8_t id);

// 速度PID回调函数
static int8_t speed_pid_set_callback(uint8_t id, float kp, float ki, float kd);
static int8_t speed_pid_get_callback(uint8_t id, float* kp, float* ki, float* kd);
static int8_t speed_pid_save_callback(uint8_t id);

/* Exported functions --------------------------------------------------------*/

/**
 * @brief PID调参库初始化示例
 * @note 在main函数或初始化阶段调用
 */
void PidTuner_Example_Init(void)
{
    // 步骤1: 创建PID调参器实例
    int8_t result = PidTuner_Create(&pid_tuner, &huart1);
    if (result != PID_TUNER_OK)
    {
        // 初始化失败处理
        return;
    }
    
    // 步骤2: 注册角度PID控制器 (ID=1)
    result = PidTuner_RegisterController(&pid_tuner, 1, "angle_pid",
                                         angle_pid_set_callback,
                                         angle_pid_get_callback,
                                         angle_pid_save_callback);
    if (result != PID_TUNER_OK)
    {
        // 注册失败处理
        return;
    }
    
    // 步骤3: 注册速度PID控制器 (ID=2)
    result = PidTuner_RegisterController(&pid_tuner, 2, "speed_pid",
                                         speed_pid_set_callback,
                                         speed_pid_get_callback,
                                         speed_pid_save_callback);
    if (result != PID_TUNER_OK)
    {
        // 注册失败处理
        return;
    }
}

/**
 * @brief 串口任务集成示例
 * @note 在现有的Uart_Task函数中添加此调用
 */
void PidTuner_Example_UartTask(void)
{
    // 假设uart_data_buffer包含接收到的命令
    extern uint8_t uart_data_buffer[BUFFER_SIZE];
    extern struct rt_ringbuffer ring_buffer;
    
    uint16_t uart_data_len = rt_ringbuffer_data_len(&ring_buffer);
    if(uart_data_len > 0)
    {
        rt_ringbuffer_get(&ring_buffer, uart_data_buffer, uart_data_len);
        uart_data_buffer[uart_data_len] = '\0';
        
        // 步骤4: 调用PID调参解析函数
        PidTuner_ParseCommand(&pid_tuner, (char*)uart_data_buffer);
        
        memset(uart_data_buffer, 0, uart_data_len);
    }
}

/* Private functions ---------------------------------------------------------*/

/**
 * @brief 角度PID参数设置回调函数
 * @param id: PID控制器ID (应该是1)
 * @param kp: 比例系数
 * @param ki: 积分系数
 * @param kd: 微分系数
 * @retval 0: 成功, -1: 失败
 */
static int8_t angle_pid_set_callback(uint8_t id, float kp, float ki, float kd)
{
    if (id != 1) return -1;  // ID验证
    
    // 更新角度PID参数
    pid_set_params(&angle_pid, kp, ki, kd);
    
    return 0;  // 成功
}

/**
 * @brief 角度PID参数获取回调函数
 * @param id: PID控制器ID (应该是1)
 * @param kp: 比例系数指针
 * @param ki: 积分系数指针
 * @param kd: 微分系数指针
 * @retval 0: 成功, -1: 失败
 */
static int8_t angle_pid_get_callback(uint8_t id, float* kp, float* ki, float* kd)
{
    if (id != 1 || !kp || !ki || !kd) return -1;
    
    // 获取当前角度PID参数
    *kp = angle_pid.kp;
    *ki = angle_pid.ki;
    *kd = angle_pid.kd;
    
    return 0;
}

/**
 * @brief 角度PID参数保存回调函数
 * @param id: PID控制器ID (应该是1)
 * @retval 0: 成功, -1: 失败
 */
static int8_t angle_pid_save_callback(uint8_t id)
{
    if (id != 1) return -1;
    
    // 保存参数到EEPROM或Flash (用户自行实现)
    // save_angle_pid_to_flash(&angle_pid);
    
    return 0;
}

/**
 * @brief 速度PID参数设置回调函数
 * @param id: PID控制器ID (应该是2)
 * @param kp: 比例系数
 * @param ki: 积分系数
 * @param kd: 微分系数
 * @retval 0: 成功, -1: 失败
 */
static int8_t speed_pid_set_callback(uint8_t id, float kp, float ki, float kd)
{
    if (id != 2) return -1;  // ID验证
    
    // 更新速度PID参数
    pid_set_params(&speed_pid, kp, ki, kd);
    
    return 0;  // 成功
}

/**
 * @brief 速度PID参数获取回调函数
 * @param id: PID控制器ID (应该是2)
 * @param kp: 比例系数指针
 * @param ki: 积分系数指针
 * @param kd: 微分系数指针
 * @retval 0: 成功, -1: 失败
 */
static int8_t speed_pid_get_callback(uint8_t id, float* kp, float* ki, float* kd)
{
    if (id != 2 || !kp || !ki || !kd) return -1;
    
    // 获取当前速度PID参数
    *kp = speed_pid.kp;
    *ki = speed_pid.ki;
    *kd = speed_pid.kd;
    
    return 0;
}

/**
 * @brief 速度PID参数保存回调函数
 * @param id: PID控制器ID (应该是2)
 * @retval 0: 成功, -1: 失败
 */
static int8_t speed_pid_save_callback(uint8_t id)
{
    if (id != 2) return -1;
    
    // 保存参数到EEPROM或Flash (用户自行实现)
    // save_speed_pid_to_flash(&speed_pid);
    
    return 0;
}

/* 命令使用示例:
 * 
 * 1. 设置角度PID参数:
 *    SET_PID,1,0.5,0.1,0.2
 *    响应: OK,SET_PID,PID[1] params updated
 * 
 * 2. 获取角度PID参数:
 *    GET_PID,1
 *    响应: OK,GET_PID,1,0.500,0.100,0.200
 * 
 * 3. 保存角度PID参数:
 *    SAVE_PID,1
 *    响应: OK,SAVE_PID,PID[1] params saved
 * 
 * 4. 列出所有PID控制器:
 *    LIST_PID
 *    响应: OK,LIST_PID,2 controllers found
 *          OK,PID_INFO,1,angle_pid,0.500,0.100,0.200
 *          OK,PID_INFO,2,speed_pid,1.000,0.050,0.010
 */

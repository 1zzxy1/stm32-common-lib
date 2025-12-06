/**
 ******************************************************************************
 * @file    pid_tuner.h
 * @brief   PID调参库头文件
 * @author  XiFeng
 * @date    2025-07-14
 ******************************************************************************
 * @attention
 * 
 * 本库专为STM32F4系列嵌入式系统设计
 * 支持通过串口助手实时调节PID参数，采用回调机制实现低耦合设计
 * 用户只需接入解析函数即可完成集成，支持多PID控制器管理
 * 
 ******************************************************************************
 */

#ifndef __PID_TUNER_H__
#define __PID_TUNER_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "MyDefine.h"

/* Exported types ------------------------------------------------------------*/

/**
 * @brief 命令类型枚举
 */
typedef enum {
    CMD_SET_PID = 0,    // 设置PID参数命令
    CMD_GET_PID,        // 获取PID参数命令
    CMD_SAVE_PID,       // 保存PID参数命令
    CMD_LIST_PID,       // 列出PID控制器命令
    CMD_UNKNOWN         // 未知命令
} PidCommand_t;

/**
 * @brief PID参数设置回调函数类型
 * @param id: PID控制器ID
 * @param kp: 比例系数
 * @param ki: 积分系数
 * @param kd: 微分系数
 * @retval 0: 成功, -1: 失败
 */
typedef int8_t (*PidSetCallback_t)(uint8_t id, float kp, float ki, float kd);

/**
 * @brief PID参数获取回调函数类型
 * @param id: PID控制器ID
 * @param kp: 比例系数指针
 * @param ki: 积分系数指针
 * @param kd: 微分系数指针
 * @retval 0: 成功, -1: 失败
 */
typedef int8_t (*PidGetCallback_t)(uint8_t id, float* kp, float* ki, float* kd);

/**
 * @brief PID参数保存回调函数类型
 * @param id: PID控制器ID
 * @retval 0: 成功, -1: 失败
 */
typedef int8_t (*PidSaveCallback_t)(uint8_t id);

/**
 * @brief PID控制器信息结构体
 */
typedef struct {
    uint8_t id;                    // 控制器ID (1-255)
    char name[16];                 // 控制器名称
    PidSetCallback_t set_callback; // 参数设置回调函数
    PidGetCallback_t get_callback; // 参数获取回调函数
    PidSaveCallback_t save_callback; // 参数保存回调函数
    uint8_t active;                // 激活状态 (0-未激活, 1-已激活)
} PidController_t;

#define MAX_PID_CONTROLLERS     8       // 最大支持的PID控制器数量
#define RESPONSE_BUFFER_SIZE    128     // 响应消息缓冲区大小
#define PID_NAME_MAX_LEN        15      // PID控制器名称最大长度

/**
 * @brief PID调参器管理结构体
 */
typedef struct {
    PidController_t controllers[MAX_PID_CONTROLLERS]; // 注册的控制器数组
    uint8_t controller_count;      // 已注册控制器数量
    char response_buffer[RESPONSE_BUFFER_SIZE]; // 响应消息缓冲区
    UART_HandleTypeDef* huart;     // 串口句柄
} PidTuner_t;

/* Exported constants --------------------------------------------------------*/
// 错误码定义
#define PID_TUNER_OK            0       // 成功
#define PID_TUNER_ERROR         -1      // 一般错误
#define PID_TUNER_PARAM_ERROR   -2      // 参数错误
#define PID_TUNER_ID_EXISTS     -3      // ID已存在
#define PID_TUNER_MAX_REACHED   -4      // 达到最大数量
#define PID_TUNER_NOT_FOUND     -5      // 控制器未找到
#define PID_TUNER_CMD_ERROR     -6      // 命令格式错误
#define PID_TUNER_EXEC_ERROR    -7      // 执行错误

// PID参数范围限制
#define PID_PARAM_MIN           -1000.0f   // PID参数最小值
#define PID_PARAM_MAX           1000.0f    // PID参数最大值

/* Exported macros -----------------------------------------------------------*/

/* Exported functions prototypes ---------------------------------------------*/

/**
 * @brief 创建PID调参器实例
 * @param tuner: 调参器实例指针
 * @param huart: 串口句柄指针
 * @retval 0: 成功, -1: 参数错误
 * @note 使用示例:
 *       PidTuner_t pid_tuner;
 *       PidTuner_Create(&pid_tuner, &huart1);
 */
int8_t PidTuner_Create(PidTuner_t* tuner, UART_HandleTypeDef* huart);

/**
 * @brief 注册PID控制器
 * @param tuner: 调参器实例指针
 * @param id: 控制器ID (1-255)
 * @param name: 控制器名称
 * @param set_cb: 参数设置回调函数
 * @param get_cb: 参数获取回调函数
 * @param save_cb: 参数保存回调函数
 * @retval 0: 成功, -2: 参数错误, -3: ID已存在, -4: 数量超限
 * @note 使用示例:
 *       PidTuner_RegisterController(&tuner, 1, "angle_pid",
 *                                   angle_set_cb, angle_get_cb, angle_save_cb);
 */
int8_t PidTuner_RegisterController(PidTuner_t* tuner, uint8_t id, const char* name,
                                   PidSetCallback_t set_cb, PidGetCallback_t get_cb,
                                   PidSaveCallback_t save_cb);

/**
 * @brief 解析命令类型
 * @param cmd: 命令字符串
 * @retval 命令类型枚举值
 */
PidCommand_t PidTuner_ParseCommandType(const char* cmd);

/**
 * @brief 解析SET_PID命令参数
 * @param cmd: 命令字符串
 * @param id: 输出PID控制器ID
 * @param kp: 输出比例系数
 * @param ki: 输出积分系数
 * @param kd: 输出微分系数
 * @retval 0: 成功, -1: 解析失败
 */
int8_t PidTuner_ParseSetPidParams(const char* cmd, uint8_t* id, float* kp, float* ki, float* kd);

/**
 * @brief 解析单ID命令参数（GET_PID、SAVE_PID）
 * @param cmd: 命令字符串
 * @param id: 输出PID控制器ID
 * @retval 0: 成功, -1: 解析失败
 */
int8_t PidTuner_ParseSingleId(const char* cmd, uint8_t* id);

/**
 * @brief 执行SET_PID命令
 * @param tuner: 调参器实例指针
 * @param id: PID控制器ID
 * @param kp: 比例系数
 * @param ki: 积分系数
 * @param kd: 微分系数
 * @retval 0: 成功, -1: 失败
 */
int8_t PidTuner_ExecuteSetPid(PidTuner_t* tuner, uint8_t id, float kp, float ki, float kd);

/**
 * @brief 执行GET_PID命令
 * @param tuner: 调参器实例指针
 * @param id: PID控制器ID
 * @retval 0: 成功, -1: 失败
 */
int8_t PidTuner_ExecuteGetPid(PidTuner_t* tuner, uint8_t id);

/**
 * @brief 执行SAVE_PID命令
 * @param tuner: 调参器实例指针
 * @param id: PID控制器ID
 * @retval 0: 成功, -1: 失败
 */
int8_t PidTuner_ExecuteSavePid(PidTuner_t* tuner, uint8_t id);

/**
 * @brief 执行LIST_PID命令
 * @param tuner: 调参器实例指针
 * @retval 0: 成功, -1: 失败
 */
int8_t PidTuner_ExecuteListPid(PidTuner_t* tuner);

/**
 * @brief 解析并执行串口命令 (用户主要调用接口)
 * @param tuner: 调参器实例指针
 * @param command: 接收到的命令字符串
 * @retval 0: 成功, -1: 参数错误, -2: 命令格式错误, -3: 执行失败
 * @note 这是用户唯一需要调用的主接口函数
 * @note 支持的命令格式:
 *       SET_PID,id,kp,ki,kd  - 设置PID参数
 *       GET_PID,id           - 获取PID参数
 *       SAVE_PID,id          - 保存PID参数
 *       LIST_PID             - 列出所有PID控制器
 * @note 使用示例:
 *       PidTuner_ParseCommand(&tuner, "SET_PID,1,0.5,0.1,0.2");
 *       PidTuner_ParseCommand(&tuner, "GET_PID,1");
 */
int8_t PidTuner_ParseCommand(PidTuner_t* tuner, const char* command);

#ifdef __cplusplus
}
#endif

#endif /* __PID_TUNER_H__ */

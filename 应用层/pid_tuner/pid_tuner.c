/**
 ******************************************************************************
 * @file    pid_tuner.c
 * @brief   PID调参库实现文件
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

/* Includes ------------------------------------------------------------------*/
#include "pid_tuner.h"
#include "uart_driver.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
static char* trim_whitespace(char* str);
static int8_t validate_pid_params(float kp, float ki, float kd);
static PidController_t* find_controller_by_id(PidTuner_t* tuner, uint8_t id);
static void send_response(PidTuner_t* tuner, const char* format, ...);

/* Private functions ---------------------------------------------------------*/

/**
 * @brief 去除字符串首尾空白字符
 * @param str: 输入字符串
 * @retval 处理后的字符串指针
 */
static char* trim_whitespace(char* str)
{
    char* end;

    // 去除前导空白
    while (*str == ' ' || *str == '\t' || *str == '\r' || *str == '\n')
        str++;

    if (*str == 0)  // 全是空白字符
        return str;

    // 去除尾部空白
    end = str + strlen(str) - 1;
    while (end > str && (*end == ' ' || *end == '\t' || *end == '\r' || *end == '\n'))
        end--;

    // 添加字符串结束符
    end[1] = '\0';

    return str;
}

/**
 * @brief 验证PID参数有效性
 * @param kp: 比例系数
 * @param ki: 积分系数
 * @param kd: 微分系数
 * @retval 0: 有效, -1: 无效
 */
static int8_t validate_pid_params(float kp, float ki, float kd)
{
    if (kp < PID_PARAM_MIN || kp > PID_PARAM_MAX ||
        ki < PID_PARAM_MIN || ki > PID_PARAM_MAX ||
        kd < PID_PARAM_MIN || kd > PID_PARAM_MAX)
    {
        return -1;
    }
    return 0;
}

/**
 * @brief 根据ID查找PID控制器
 * @param tuner: 调参器实例指针
 * @param id: 控制器ID
 * @retval 控制器指针，未找到返回NULL
 */
static PidController_t* find_controller_by_id(PidTuner_t* tuner, uint8_t id)
{
    if (tuner == NULL)
        return NULL;

    for (uint8_t i = 0; i < tuner->controller_count; i++)
    {
        if (tuner->controllers[i].id == id && tuner->controllers[i].active)
        {
            return &tuner->controllers[i];
        }
    }
    return NULL;
}

/**
 * @brief 发送响应消息
 * @param tuner: 调参器实例指针
 * @param format: 格式化字符串
 * @param ...: 可变参数
 */
static void send_response(PidTuner_t* tuner, const char* format, ...)
{
    if (tuner == NULL || tuner->huart == NULL || format == NULL)
        return;

    va_list args;
    va_start(args, format);
    vsnprintf(tuner->response_buffer, RESPONSE_BUFFER_SIZE, format, args);
    va_end(args);

    // 确保字符串结束
    tuner->response_buffer[RESPONSE_BUFFER_SIZE - 1] = '\0';

    // 发送响应
    Uart_Printf(tuner->huart, "%s\r\n", tuner->response_buffer);
}

/* Exported functions --------------------------------------------------------*/

/**
 * @brief 创建PID调参器实例
 * @param tuner: 调参器实例指针
 * @param huart: 串口句柄指针
 * @retval 0: 成功, -1: 参数错误
 */
int8_t PidTuner_Create(PidTuner_t* tuner, UART_HandleTypeDef* huart)
{
    // 参数有效性检查
    if (tuner == NULL || huart == NULL)
    {
        return PID_TUNER_PARAM_ERROR;
    }

    // 初始化控制器数组（清零）
    memset(tuner->controllers, 0, sizeof(tuner->controllers));

    // 设置控制器计数为0
    tuner->controller_count = 0;

    // 清空响应缓冲区
    memset(tuner->response_buffer, 0, sizeof(tuner->response_buffer));

    // 保存串口句柄
    tuner->huart = huart;

    return PID_TUNER_OK;
}

/**
 * @brief 注册PID控制器
 * @param tuner: 调参器实例指针
 * @param id: 控制器ID (1-255)
 * @param name: 控制器名称
 * @param set_cb: 参数设置回调函数
 * @param get_cb: 参数获取回调函数
 * @param save_cb: 参数保存回调函数
 * @retval 0: 成功, -2: 参数错误, -3: ID已存在, -4: 数量超限
 */
int8_t PidTuner_RegisterController(PidTuner_t* tuner, uint8_t id, const char* name,
                                   PidSetCallback_t set_cb, PidGetCallback_t get_cb,
                                   PidSaveCallback_t save_cb)
{
    // 参数有效性检查
    if (tuner == NULL || name == NULL || set_cb == NULL || get_cb == NULL || save_cb == NULL)
    {
        return PID_TUNER_PARAM_ERROR;
    }

    // 检查ID有效性 (1-255)
    if (id == 0)
    {
        return PID_TUNER_PARAM_ERROR;
    }

    // 检查是否达到最大注册数量
    if (tuner->controller_count >= MAX_PID_CONTROLLERS)
    {
        return PID_TUNER_MAX_REACHED;
    }

    // 检查ID是否已存在（避免重复注册）
    for (uint8_t i = 0; i < tuner->controller_count; i++)
    {
        if (tuner->controllers[i].id == id)
        {
            return PID_TUNER_ID_EXISTS;
        }
    }

    // 在控制器数组中添加新控制器
    PidController_t* controller = &tuner->controllers[tuner->controller_count];

    // 设置控制器信息
    controller->id = id;

    // 复制控制器名称（使用strncpy确保安全）
    strncpy(controller->name, name, PID_NAME_MAX_LEN);
    controller->name[PID_NAME_MAX_LEN] = '\0';  // 确保字符串结束

    // 设置回调函数指针
    controller->set_callback = set_cb;
    controller->get_callback = get_cb;
    controller->save_callback = save_cb;

    // 设置激活状态
    controller->active = 1;

    // 增加控制器计数
    tuner->controller_count++;

    return PID_TUNER_OK;
}

/**
 * @brief 解析命令类型
 * @param cmd: 命令字符串
 * @retval 命令类型枚举值
 */
PidCommand_t PidTuner_ParseCommandType(const char* cmd)
{
    if (cmd == NULL)
        return CMD_UNKNOWN;

    // 查找第一个逗号或字符串结束
    const char* comma = strchr(cmd, ',');
    size_t cmd_len = comma ? (size_t)(comma - cmd) : strlen(cmd);

    // 比较命令类型
    if (strncmp(cmd, "SET_PID", cmd_len) == 0 && cmd_len == 7)
        return CMD_SET_PID;
    else if (strncmp(cmd, "GET_PID", cmd_len) == 0 && cmd_len == 7)
        return CMD_GET_PID;
    else if (strncmp(cmd, "SAVE_PID", cmd_len) == 0 && cmd_len == 8)
        return CMD_SAVE_PID;
    else if (strncmp(cmd, "LIST_PID", cmd_len) == 0 && cmd_len == 8)
        return CMD_LIST_PID;

    return CMD_UNKNOWN;
}

/**
 * @brief 解析SET_PID命令参数
 * @param cmd: 命令字符串 (格式: SET_PID,id,kp,ki,kd)
 * @param id: 输出PID控制器ID
 * @param kp: 输出比例系数
 * @param ki: 输出积分系数
 * @param kd: 输出微分系数
 * @retval 0: 成功, -1: 解析失败
 */
int8_t PidTuner_ParseSetPidParams(const char* cmd, uint8_t* id, float* kp, float* ki, float* kd)
{
    if (cmd == NULL || id == NULL || kp == NULL || ki == NULL || kd == NULL)
        return -1;

    // 创建可修改的副本
    char cmd_copy[64];
    strncpy(cmd_copy, cmd, sizeof(cmd_copy) - 1);
    cmd_copy[sizeof(cmd_copy) - 1] = '\0';

    // 使用strtok分割字符串
    char* token = strtok(cmd_copy, ",");
    if (token == NULL || strcmp(token, "SET_PID") != 0)
        return -1;

    // 解析ID
    token = strtok(NULL, ",");
    if (token == NULL)
        return -1;
    int temp_id = atoi(trim_whitespace(token));
    if (temp_id <= 0 || temp_id > 255)
        return -1;
    *id = (uint8_t)temp_id;

    // 解析kp
    token = strtok(NULL, ",");
    if (token == NULL)
        return -1;
    *kp = atof(trim_whitespace(token));

    // 解析ki
    token = strtok(NULL, ",");
    if (token == NULL)
        return -1;
    *ki = atof(trim_whitespace(token));

    // 解析kd
    token = strtok(NULL, ",");
    if (token == NULL)
        return -1;
    *kd = atof(trim_whitespace(token));

    // 验证参数范围
    if (validate_pid_params(*kp, *ki, *kd) != 0)
        return -1;

    return 0;
}

/**
 * @brief 解析单ID命令参数（GET_PID、SAVE_PID）
 * @param cmd: 命令字符串 (格式: GET_PID,id 或 SAVE_PID,id)
 * @param id: 输出PID控制器ID
 * @retval 0: 成功, -1: 解析失败
 */
int8_t PidTuner_ParseSingleId(const char* cmd, uint8_t* id)
{
    if (cmd == NULL || id == NULL)
        return -1;

    // 创建可修改的副本
    char cmd_copy[32];
    strncpy(cmd_copy, cmd, sizeof(cmd_copy) - 1);
    cmd_copy[sizeof(cmd_copy) - 1] = '\0';

    // 使用strtok分割字符串
    char* token = strtok(cmd_copy, ",");
    if (token == NULL)
        return -1;

    // 验证命令类型
    if (strcmp(token, "GET_PID") != 0 && strcmp(token, "SAVE_PID") != 0)
        return -1;

    // 解析ID
    token = strtok(NULL, ",");
    if (token == NULL)
        return -1;

    int temp_id = atoi(trim_whitespace(token));
    if (temp_id <= 0 || temp_id > 255)
        return -1;

    *id = (uint8_t)temp_id;
    return 0;
}

/**
 * @brief 执行SET_PID命令
 * @param tuner: 调参器实例指针
 * @param id: PID控制器ID
 * @param kp: 比例系数
 * @param ki: 积分系数
 * @param kd: 微分系数
 * @retval 0: 成功, -1: 失败
 */
int8_t PidTuner_ExecuteSetPid(PidTuner_t* tuner, uint8_t id, float kp, float ki, float kd)
{
    if (tuner == NULL)
    {
        return PID_TUNER_PARAM_ERROR;
    }

    // 查找对应的PID控制器
    PidController_t* controller = find_controller_by_id(tuner, id);
    if (controller == NULL)
    {
        send_response(tuner, "ERROR,%d,PID controller not found", PID_TUNER_NOT_FOUND);
        return PID_TUNER_NOT_FOUND;
    }

    // 检查回调函数有效性
    if (controller->set_callback == NULL)
    {
        send_response(tuner, "ERROR,%d,Set callback not available", PID_TUNER_EXEC_ERROR);
        return PID_TUNER_EXEC_ERROR;
    }

    // 调用set_callback回调函数
    int8_t result = controller->set_callback(id, kp, ki, kd);

    // 处理回调返回值并生成响应
    if (result == 0)
    {
        send_response(tuner, "OK,SET_PID,PID[%d] params updated", id);
        return PID_TUNER_OK;
    }
    else
    {
        send_response(tuner, "ERROR,%d,Failed to set PID parameters", PID_TUNER_EXEC_ERROR);
        return PID_TUNER_EXEC_ERROR;
    }
}

/**
 * @brief 执行GET_PID命令
 * @param tuner: 调参器实例指针
 * @param id: PID控制器ID
 * @retval 0: 成功, -1: 失败
 */
int8_t PidTuner_ExecuteGetPid(PidTuner_t* tuner, uint8_t id)
{
    if (tuner == NULL)
    {
        return PID_TUNER_PARAM_ERROR;
    }

    // 查找对应的PID控制器
    PidController_t* controller = find_controller_by_id(tuner, id);
    if (controller == NULL)
    {
        send_response(tuner, "ERROR,%d,PID controller not found", PID_TUNER_NOT_FOUND);
        return PID_TUNER_NOT_FOUND;
    }

    // 检查回调函数有效性
    if (controller->get_callback == NULL)
    {
        send_response(tuner, "ERROR,%d,Get callback not available", PID_TUNER_EXEC_ERROR);
        return PID_TUNER_EXEC_ERROR;
    }

    // 调用get_callback获取当前参数
    float kp, ki, kd;
    int8_t result = controller->get_callback(id, &kp, &ki, &kd);

    // 处理回调返回值并生成响应
    if (result == 0)
    {
        send_response(tuner, "OK,GET_PID,%d,%.3f,%.3f,%.3f", id, kp, ki, kd);
        return PID_TUNER_OK;
    }
    else
    {
        send_response(tuner, "ERROR,%d,Failed to get PID parameters", PID_TUNER_EXEC_ERROR);
        return PID_TUNER_EXEC_ERROR;
    }
}

/**
 * @brief 执行SAVE_PID命令
 * @param tuner: 调参器实例指针
 * @param id: PID控制器ID
 * @retval 0: 成功, -1: 失败
 */
int8_t PidTuner_ExecuteSavePid(PidTuner_t* tuner, uint8_t id)
{
    if (tuner == NULL)
    {
        return PID_TUNER_PARAM_ERROR;
    }

    // 查找对应的PID控制器
    PidController_t* controller = find_controller_by_id(tuner, id);
    if (controller == NULL)
    {
        send_response(tuner, "ERROR,%d,PID controller not found", PID_TUNER_NOT_FOUND);
        return PID_TUNER_NOT_FOUND;
    }

    // 检查回调函数有效性
    if (controller->save_callback == NULL)
    {
        send_response(tuner, "ERROR,%d,Save callback not available", PID_TUNER_EXEC_ERROR);
        return PID_TUNER_EXEC_ERROR;
    }

    // 调用save_callback保存参数
    int8_t result = controller->save_callback(id);

    // 处理回调返回值并生成响应
    if (result == 0)
    {
        send_response(tuner, "OK,SAVE_PID,PID[%d] params saved", id);
        return PID_TUNER_OK;
    }
    else
    {
        send_response(tuner, "ERROR,%d,Failed to save PID parameters", PID_TUNER_EXEC_ERROR);
        return PID_TUNER_EXEC_ERROR;
    }
}

/**
 * @brief 执行LIST_PID命令
 * @param tuner: 调参器实例指针
 * @retval 0: 成功, -1: 失败
 */
int8_t PidTuner_ExecuteListPid(PidTuner_t* tuner)
{
    if (tuner == NULL)
    {
        return PID_TUNER_PARAM_ERROR;
    }

    // 检查是否有注册的控制器
    if (tuner->controller_count == 0)
    {
        send_response(tuner, "OK,LIST_PID,No PID controllers registered");
        return PID_TUNER_OK;
    }

    // 发送控制器数量
    send_response(tuner, "OK,LIST_PID,%d controllers found", tuner->controller_count);

    // 遍历所有注册的控制器
    for (uint8_t i = 0; i < tuner->controller_count; i++)
    {
        PidController_t* controller = &tuner->controllers[i];
        if (controller->active)
        {
            // 获取当前参数
            float kp = 0, ki = 0, kd = 0;
            if (controller->get_callback != NULL)
            {
                controller->get_callback(controller->id, &kp, &ki, &kd);
            }

            // 发送控制器信息
            send_response(tuner, "OK,PID_INFO,%d,%s,%.3f,%.3f,%.3f",
                          controller->id, controller->name, kp, ki, kd);
        }
    }

    return PID_TUNER_OK;
}

/**
 * @brief 解析并执行串口命令 (用户主要调用接口)
 * @param tuner: 调参器实例指针
 * @param command: 接收到的命令字符串
 * @retval 0: 成功, -1: 参数错误, -2: 命令格式错误, -3: 执行失败
 */
int8_t PidTuner_ParseCommand(PidTuner_t* tuner, const char* command)
{
    // 参数有效性检查
    if (tuner == NULL || command == NULL)
    {
        return PID_TUNER_PARAM_ERROR;
    }

    // 创建可修改的命令副本并去除空白字符
    char cmd_copy[128];
    strncpy(cmd_copy, command, sizeof(cmd_copy) - 1);
    cmd_copy[sizeof(cmd_copy) - 1] = '\0';
    char* trimmed_cmd = trim_whitespace(cmd_copy);

    // 检查命令是否为空
    if (strlen(trimmed_cmd) == 0)
    {
        send_response(tuner, "ERROR,%d,Empty command", PID_TUNER_CMD_ERROR);
        return PID_TUNER_CMD_ERROR;
    }

    // 调用命令解析器识别命令类型
    PidCommand_t cmd_type = PidTuner_ParseCommandType(trimmed_cmd);

    // 根据命令类型分发到相应的执行函数
    int8_t result = PID_TUNER_OK;

    switch (cmd_type)
    {
        case CMD_SET_PID:
        {
            uint8_t id;
            float kp, ki, kd;

            // 解析SET_PID参数
            if (PidTuner_ParseSetPidParams(trimmed_cmd, &id, &kp, &ki, &kd) != 0)
            {
                send_response(tuner, "ERROR,%d,Invalid SET_PID parameters", PID_TUNER_CMD_ERROR);
                return PID_TUNER_CMD_ERROR;
            }

            // 执行SET_PID命令
            result = PidTuner_ExecuteSetPid(tuner, id, kp, ki, kd);
            break;
        }

        case CMD_GET_PID:
        {
            uint8_t id;

            // 解析GET_PID参数
            if (PidTuner_ParseSingleId(trimmed_cmd, &id) != 0)
            {
                send_response(tuner, "ERROR,%d,Invalid GET_PID parameters", PID_TUNER_CMD_ERROR);
                return PID_TUNER_CMD_ERROR;
            }

            // 执行GET_PID命令
            result = PidTuner_ExecuteGetPid(tuner, id);
            break;
        }

        case CMD_SAVE_PID:
        {
            uint8_t id;

            // 解析SAVE_PID参数
            if (PidTuner_ParseSingleId(trimmed_cmd, &id) != 0)
            {
                send_response(tuner, "ERROR,%d,Invalid SAVE_PID parameters", PID_TUNER_CMD_ERROR);
                return PID_TUNER_CMD_ERROR;
            }

            // 执行SAVE_PID命令
            result = PidTuner_ExecuteSavePid(tuner, id);
            break;
        }

        case CMD_LIST_PID:
        {
            // 执行LIST_PID命令（无需参数）
            result = PidTuner_ExecuteListPid(tuner);
            break;
        }

        case CMD_UNKNOWN:
        default:
        {
            send_response(tuner, "ERROR,%d,Unknown command", PID_TUNER_CMD_ERROR);
            return PID_TUNER_CMD_ERROR;
        }
    }

    return result;
}

/**
 ******************************************************************************
 * @file    pid.h
 * @brief   通用PID控制器库
 * @author  XiFeng
 * @version 1.0.0
 ******************************************************************************
 * @attention
 *
 * 支持位置式PID和增量式PID两种算法
 * 纯C实现，无硬件依赖，可用于任何嵌入式平台
 *
 ******************************************************************************
 */

#ifndef __PID_H
#define __PID_H

#ifdef __cplusplus
extern "C" {
#endif

/* PID结构体 */
typedef struct
{
    float kp;               /* 比例系数 */
    float ki;               /* 积分系数 */
    float kd;               /* 微分系数 */
    float target;           /* 目标值 */
    float current;          /* 当前值 */
    float out;              /* 执行量(输出) */
    float limit;            /* PID输出限幅值 */

    float error;            /* 当前误差 */
    float last_error;       /* 上一次误差 */
    float last2_error;      /* 上上次误差 */
    float last_out;         /* 上一次执行量 */
    float integral;         /* 积分项（累加） */
    float p_out, i_out, d_out;  /* 比例、积分、微分分量 */
} PID_T;

/**
 * @brief PID初始化
 * @param _tpPID 指向PID结构体的指针
 * @param _kp 比例系数
 * @param _ki 积分系数
 * @param _kd 微分系数
 * @param _target 目标值
 * @param _limit 输出限幅值
 */
void pid_init(PID_T *_tpPID, float _kp, float _ki, float _kd, float _target, float _limit);

/**
 * @brief 设置PID目标值
 * @param _tpPID 指向PID结构体的指针
 * @param _target 目标值
 */
void pid_set_target(PID_T *_tpPID, float _target);

/**
 * @brief 设置PID参数
 * @param _tpPID 指向PID结构体的指针
 * @param _kp 比例系数
 * @param _ki 积分系数
 * @param _kd 微分系数
 */
void pid_set_params(PID_T *_tpPID, float _kp, float _ki, float _kd);

/**
 * @brief 设置PID输出限幅
 * @param _tpPID 指向PID结构体的指针
 * @param _limit 限幅值
 */
void pid_set_limit(PID_T *_tpPID, float _limit);

/**
 * @brief 重置PID控制器
 * @param _tpPID 指向PID结构体的指针
 * @note 清除所有历史误差数据
 */
void pid_reset(PID_T *_tpPID);

/**
 * @brief 计算位置式PID
 * @param _tpPID 指向PID结构体的指针
 * @param _current 当前值
 * @return PID计算后的输出值
 * @note 位置式PID：P-响应性，I-准确性，D-稳定性
 */
float pid_calculate_positional(PID_T *_tpPID, float _current);

/**
 * @brief 计算增量式PID
 * @param _tpPID 指向PID结构体的指针
 * @param _current 当前值
 * @return PID计算后的输出值
 * @note 增量式PID：P-稳定性，I-响应性，D-准确性
 */
float pid_calculate_incremental(PID_T *_tpPID, float _current);

/**
 * @brief 通用限幅函数
 * @param value 输入值
 * @param min 最小值
 * @param max 最大值
 * @return 限幅后的值
 */
float pid_constrain(float value, float min, float max);

/**
 * @brief 积分限幅函数
 * @param pid PID控制器
 * @param min 最小值
 * @param max 最大值
 * @note 用于位置式PID防止积分饱和
 */
void pid_limit_integral(PID_T *pid, float min, float max);

#ifdef __cplusplus
}
#endif

#endif /* __PID_H */

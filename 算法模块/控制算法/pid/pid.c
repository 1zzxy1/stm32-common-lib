/**
 ******************************************************************************
 * @file    pid.c
 * @brief   通用PID控制器库实现
 * @author  XiFeng
 * @version 1.0.0
 ******************************************************************************
 */

#include <stdint.h>
#include <stdbool.h>
#include "pid.h"

/* 内部功能函数 */
static void pid_formula_incremental(PID_T *_tpPID);
static void pid_formula_positional(PID_T *_tpPID);
static void pid_out_limit(PID_T *_tpPID);

/**
 * @brief PID初始化函数
 */
void pid_init(PID_T *_tpPID, float _kp, float _ki, float _kd, float _target, float _limit)
{
    _tpPID->kp = _kp;
    _tpPID->ki = _ki;
    _tpPID->kd = _kd;
    _tpPID->target = _target;
    _tpPID->limit = _limit;
    _tpPID->integral = 0;
    _tpPID->last_error = 0;
    _tpPID->last2_error = 0;
    _tpPID->out = 0;
    _tpPID->p_out = 0;
    _tpPID->i_out = 0;
    _tpPID->d_out = 0;
}

/**
 * @brief 设置PID目标值
 */
void pid_set_target(PID_T *_tpPID, float _target)
{
    _tpPID->target = _target;
    pid_init(_tpPID,
             _tpPID->kp, _tpPID->ki, _tpPID->kd,
             _tpPID->target, _tpPID->limit);
}

/**
 * @brief 设置PID参数
 */
void pid_set_params(PID_T *_tpPID, float _kp, float _ki, float _kd)
{
    _tpPID->kp = _kp;
    _tpPID->ki = _ki;
    _tpPID->kd = _kd;
}

/**
 * @brief 设置PID输出限幅
 */
void pid_set_limit(PID_T *_tpPID, float _limit)
{
    _tpPID->limit = _limit;
}

/**
 * @brief 重置PID控制器
 */
void pid_reset(PID_T *_tpPID)
{
    _tpPID->integral = 0;
    _tpPID->last_error = 0;
    _tpPID->last2_error = 0;
    _tpPID->out = 0;
    _tpPID->p_out = 0;
    _tpPID->i_out = 0;
    _tpPID->d_out = 0;
}

/**
 * @brief 计算位置式PID
 */
float pid_calculate_positional(PID_T *_tpPID, float _current)
{
    _tpPID->current = _current;
    pid_formula_positional(_tpPID);
    pid_out_limit(_tpPID);
    return _tpPID->out;
}

/**
 * @brief 计算增量式PID
 */
float pid_calculate_incremental(PID_T *_tpPID, float _current)
{
    _tpPID->current = _current;
    pid_formula_incremental(_tpPID);
    pid_out_limit(_tpPID);
    return _tpPID->out;
}

/* ======================= 内部函数实现 ======================= */

/**
 * @brief 输出限幅函数
 */
static void pid_out_limit(PID_T *_tpPID)
{
    if (_tpPID->out > _tpPID->limit)
        _tpPID->out = _tpPID->limit;
    else if (_tpPID->out < -_tpPID->limit)
        _tpPID->out = -_tpPID->limit;
}

/**
 * @brief 增量式PID公式
 * @note 在增量式中，P-稳定性，I-响应性，D-准确性
 */
static void pid_formula_incremental(PID_T *_tpPID)
{
    _tpPID->error = _tpPID->target - _tpPID->current;

    _tpPID->p_out = _tpPID->kp * (_tpPID->error - _tpPID->last_error);
    _tpPID->i_out = _tpPID->ki * _tpPID->error;
    _tpPID->d_out = _tpPID->kd * (_tpPID->error - 2 * _tpPID->last_error + _tpPID->last2_error);

    _tpPID->out += _tpPID->p_out + _tpPID->i_out + _tpPID->d_out;

    _tpPID->last2_error = _tpPID->last_error;
    _tpPID->last_error = _tpPID->error;
}

/**
 * @brief 位置式PID公式
 * @note 在位置式中，P-响应性，I-准确性，D-稳定性
 */
static void pid_formula_positional(PID_T *_tpPID)
{
    _tpPID->error = _tpPID->target - _tpPID->current;
    _tpPID->integral += _tpPID->error;

    _tpPID->p_out = _tpPID->kp * _tpPID->error;
    _tpPID->i_out = _tpPID->ki * _tpPID->integral;
    _tpPID->d_out = _tpPID->kd * (_tpPID->error - _tpPID->last_error);

    _tpPID->out = _tpPID->p_out + _tpPID->i_out + _tpPID->d_out;

    _tpPID->last_error = _tpPID->error;
}

/**
 * @brief 通用限幅函数
 */
float pid_constrain(float value, float min, float max)
{
    if (value < min)
        return min;
    else if (value > max)
        return max;
    else
        return value;
}

/**
 * @brief 积分限幅函数
 */
void pid_limit_integral(PID_T *pid, float min, float max)
{
    if (pid->integral > max)
    {
        pid->integral = max;
    }
    else if (pid->integral < min)
    {
        pid->integral = min;
    }
}

/**
 ******************************************************************************
 * @file    kalman.c
 * @brief   一维卡尔曼滤波器实现
 * @version 1.0.0
 ******************************************************************************
 * @note
 *
 * 卡尔曼滤波算法步骤:
 * 1. 预测(Prediction):
 *    - 状态预测: x(k|k-1) = A * x(k-1|k-1)
 *    - 协方差预测: P(k|k-1) = A * P(k-1|k-1) * A' + Q
 *
 * 2. 更新(Update):
 *    - 卡尔曼增益: K = P(k|k-1) * H' / (H * P(k|k-1) * H' + R)
 *    - 状态更新: x(k|k) = x(k|k-1) + K * (z(k) - H * x(k|k-1))
 *    - 协方差更新: P(k|k) = (I - K * H) * P(k|k-1)
 *
 ******************************************************************************
 */

#include "kalman.h"

/**
 * @brief 初始化卡尔曼滤波器
 */
void kalman_init(kalman_t *kf, float init_x, float init_p, float q, float r)
{
    if (kf == NULL) {
        return;
    }

    kf->x = init_x;     /* 初始状态估计 */
    kf->P = init_p;     /* 初始估计协方差 */
    kf->A = 1.0f;       /* 状态转移矩阵 (恒定值模型) */
    kf->H = 1.0f;       /* 观测矩阵 (直接观测) */
    kf->Q = q;          /* 过程噪声协方差 */
    kf->R = r;          /* 测量噪声协方差 */
    kf->K = 0.0f;       /* 卡尔曼增益 */
}

/**
 * @brief 使用默认参数初始化卡尔曼滤波器
 */
void kalman_init_default(kalman_t *kf, float init_x)
{
    kalman_init(kf, init_x, 1.0f, 0.01f, 0.1f);
}

/**
 * @brief 执行一次卡尔曼滤波迭代
 */
float kalman_update(kalman_t *kf, float measurement)
{
    if (kf == NULL) {
        return measurement;
    }

    /* 1. 预测阶段 */
    /* 状态预测: x(k|k-1) = A * x(k-1|k-1) */
    float x_pred = kf->A * kf->x;

    /* 协方差预测: P(k|k-1) = A * P(k-1|k-1) * A' + Q */
    float p_pred = kf->A * kf->P * kf->A + kf->Q;

    /* 2. 更新阶段 */
    /* 计算卡尔曼增益: K = P(k|k-1) * H' / (H * P(k|k-1) * H' + R) */
    float denominator = kf->H * p_pred * kf->H + kf->R;

    /* 避免除零 */
    if (denominator < 1e-10f && denominator > -1e-10f) {
        denominator = 1e-10f;
    }

    kf->K = p_pred * kf->H / denominator;

    /* 状态更新: x(k|k) = x(k|k-1) + K * (z(k) - H * x(k|k-1)) */
    float innovation = measurement - kf->H * x_pred;
    kf->x = x_pred + kf->K * innovation;

    /* 协方差更新: P(k|k) = (I - K * H) * P(k|k-1) */
    kf->P = (1.0f - kf->K * kf->H) * p_pred;

    return kf->x;
}

/**
 * @brief 重置滤波器状态
 */
void kalman_reset(kalman_t *kf, float new_x)
{
    if (kf == NULL) {
        return;
    }

    kf->x = new_x;
    kf->P = 1.0f;
    kf->K = 0.0f;
}

/**
 * @brief 设置过程噪声协方差
 */
void kalman_set_q(kalman_t *kf, float q)
{
    if (kf == NULL) {
        return;
    }
    kf->Q = q;
}

/**
 * @brief 设置测量噪声协方差
 */
void kalman_set_r(kalman_t *kf, float r)
{
    if (kf == NULL) {
        return;
    }
    kf->R = r;
}

/**
 * @brief 获取当前状态估计值
 */
float kalman_get_state(kalman_t *kf)
{
    if (kf == NULL) {
        return 0.0f;
    }
    return kf->x;
}

/**
 * @brief 获取当前卡尔曼增益
 */
float kalman_get_gain(kalman_t *kf)
{
    if (kf == NULL) {
        return 0.0f;
    }
    return kf->K;
}

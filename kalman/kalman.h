/**
 ******************************************************************************
 * @file    kalman.h
 * @brief   一维卡尔曼滤波器
 * @version 1.0.0
 ******************************************************************************
 * @attention
 *
 * 轻量级一维卡尔曼滤波器实现
 * 适用于传感器数据平滑、噪声滤除等场景
 * 纯C实现，无硬件依赖
 *
 ******************************************************************************
 */

#ifndef _KALMAN_H_
#define _KALMAN_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 卡尔曼滤波器状态结构体
 */
typedef struct {
    float x;    /* 状态估计值 x(k|k) */
    float A;    /* 状态转移矩阵 A (一维时为标量) */
    float H;    /* 观测矩阵 H (一维时为标量) */
    float Q;    /* 过程噪声协方差 Q */
    float R;    /* 测量噪声协方差 R */
    float P;    /* 估计误差协方差 P(k|k) */
    float K;    /* 卡尔曼增益 K */
} kalman_t;

/**
 * @brief 初始化卡尔曼滤波器
 * @param kf: 滤波器实例指针
 * @param init_x: 初始状态估计值
 * @param init_p: 初始估计误差协方差
 * @param q: 过程噪声协方差 (越小越信任模型预测)
 * @param r: 测量噪声协方差 (越小越信任测量值)
 */
void kalman_init(kalman_t *kf, float init_x, float init_p, float q, float r);

/**
 * @brief 使用默认参数初始化卡尔曼滤波器
 * @param kf: 滤波器实例指针
 * @param init_x: 初始状态估计值
 * @note 使用默认参数: P=1.0, Q=0.01, R=0.1
 */
void kalman_init_default(kalman_t *kf, float init_x);

/**
 * @brief 执行一次卡尔曼滤波迭代
 * @param kf: 滤波器实例指针
 * @param measurement: 当前测量值
 * @retval 滤波后的估计值
 */
float kalman_update(kalman_t *kf, float measurement);

/**
 * @brief 重置滤波器状态
 * @param kf: 滤波器实例指针
 * @param new_x: 新的状态估计值
 */
void kalman_reset(kalman_t *kf, float new_x);

/**
 * @brief 设置过程噪声协方差
 * @param kf: 滤波器实例指针
 * @param q: 新的过程噪声协方差值
 */
void kalman_set_q(kalman_t *kf, float q);

/**
 * @brief 设置测量噪声协方差
 * @param kf: 滤波器实例指针
 * @param r: 新的测量噪声协方差值
 */
void kalman_set_r(kalman_t *kf, float r);

/**
 * @brief 获取当前状态估计值
 * @param kf: 滤波器实例指针
 * @retval 当前状态估计值
 */
float kalman_get_state(kalman_t *kf);

/**
 * @brief 获取当前卡尔曼增益
 * @param kf: 滤波器实例指针
 * @retval 当前卡尔曼增益
 */
float kalman_get_gain(kalman_t *kf);

#ifdef __cplusplus
}
#endif

#endif /* _KALMAN_H_ */

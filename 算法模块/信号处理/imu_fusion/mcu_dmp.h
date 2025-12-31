#ifndef MCU_DMP_H
#define MCU_DMP_H

#include <math.h>
#include <stdint.h>

/* 定义常量 */
#define DEG2RAD		0.017453293f
#define RAD2DEG		57.29578f
#define ABS(x) 		(((x) < 0) ? (-x) : (x))

/* 定义3轴数据的结构体 */
typedef struct {
    float x;
    float y;
    float z;
} Axis3f;

/* 定义欧拉角的结构体 */
typedef struct {
    float roll;
    float pitch;
    float yaw;
} EulerAngles;

/* 定义卡尔曼滤波器结构体 */
typedef struct {
    float q;     // 过程噪声
    float r;     // 测量噪声
    float x;     // 状态估计
    float p;     // 估计误差协方差
    float k;     // 卡尔曼增益
    uint8_t init;// 初始化标志
} KalmanFilter;

typedef struct {
    float x;     // 状态估计
    float p;     // 估计误差协方差
    float q;     // 过程噪声
    float r;     // 测量噪声
    float k;     // 卡尔曼增益
    float v;     // 创新序列
    float s;     // 创新协方差
    float r0;    // 基础测量噪声
    float window[10]; // 创新窗口
    int window_idx;   // 窗口索引
    int init;    // 初始化标志
} AdaptiveKalmanFilter;

/* 函数声明 */
void imu_init(void);
void imu_update(Axis3f acc, Axis3f gyro, float dt);
EulerAngles imu_get_euler_angles(Axis3f gyro);
float invSqrt(float x);
float kalman_filter(KalmanFilter* kf, float measurement, float gyro_rate, float dt);


#endif 


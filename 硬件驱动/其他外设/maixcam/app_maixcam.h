// Copyright (c) 2024 米醋电子工作室

#ifndef __APP_MAIXCAM_H
#define __APP_MAIXCAM_H

#include "mydefine.h"

// 定义协议常量
#define FRAME_HEADER '$'
#define FRAME_FOOTER '\n'
#define RED_LASER_ID 'R'
#define GREEN_LASER_ID 'G'

// 激光坐标数据结构
#ifndef LASERCOORD_T_DEFINED
#define LASERCOORD_T_DEFINED
typedef struct {
    char type;    // 激光类型: 'R'表示红色激光，'G'表示绿色激光
    int x;        // X坐标
    int y;        // Y坐标
} LaserCoord_t;
#endif

// 定义回调函数类型
typedef void (*LaserCoordCallback_t)(LaserCoord_t coord);

// MaixCam 任务定义
void maixcam_task(MultiTimer *timer, void *userData);

// 数据解析函数
int maixcam_parse_data(char *buffer);

// 设置坐标数据回调函数
void maixcam_set_callback(LaserCoordCallback_t callback);

#endif /* __APP_MAIXCAM_H */

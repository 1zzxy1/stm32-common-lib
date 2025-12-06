// Copyright (c) 2024 米醋电子工作室

#include "app_maixcam.h"

// 默认回调函数
static void default_laser_callback(LaserCoord_t coord)
{

    // 打印解析的坐标数据
    if (coord.type == RED_LASER_ID)
    {
        // 第一次读取到坐标值
        app_pid_init();
        // 设置默认目标位置
        app_pid_set_target(100, 100);
        // 打印坐标值
        my_printf(&huart1, "R: X=%d, Y=%d\r\n", coord.x, coord.y);
        // 启动PID
        app_pid_start();
    }

    else
        my_printf(&huart1, "G: X=%d, Y=%d\r\n", coord.x, coord.y);
}

// 回调函数指针，默认指向默认回调函数
static LaserCoordCallback_t laser_coord_callback = default_laser_callback;

// 设置坐标数据回调函数
void maixcam_set_callback(LaserCoordCallback_t callback)
{
    if (callback != NULL)
        laser_coord_callback = callback;
    else
        laser_coord_callback = default_laser_callback;
}

// MaixCam 数据解析函数，解析格式：$类型,X坐标,Y坐标,校验和\n
int maixcam_parse_data(char *buffer)
{
    if (!buffer || buffer[0] != FRAME_HEADER)
        return -1; // 帧头错误

    // 找到最后一个逗号的位置，它之后是校验和
    char *last_comma = strrchr(buffer, ',');
    if (!last_comma)
        return -2; // 数据格式错误，找不到校验和分隔符

    // 计算校验和
    int calc_checksum = 0;
    char *p = buffer;
    while (p < last_comma)
    {
        calc_checksum += (unsigned char)*p;
        p++;
    }
    calc_checksum &= 0xFF; // 取低8位

    // 解析接收到的校验和
    int actual_checksum = 0;
    if (sscanf(last_comma + 1, "%X", &actual_checksum) != 1)
        return -3; // 校验和格式错误

    if (calc_checksum != actual_checksum)
        return -4; // 校验和错误

    // 解析坐标数据
    LaserCoord_t coord;
    int parsed = sscanf(buffer, "$%c,%d,%d", &coord.type, &coord.x, &coord.y);

    if (parsed != 3)
        return -5; // 解析失败

    // 检查激光类型
    if (coord.type != RED_LASER_ID && coord.type != GREEN_LASER_ID)
        return -6; // 未知的激光类型

    // 通过回调函数处理解析结果
    if (laser_coord_callback)
        laser_coord_callback(coord);

    return 0; // 成功
}

// MaixCam 数据解析任务函数
void maixcam_task(MultiTimer *timer, void *userData)
{
    int length_cam = rt_ringbuffer_data_len(&ringbuffer_cam);
    if (length_cam > 0)
    {
        rt_ringbuffer_get(&ringbuffer_cam, output_buffer_cam, length_cam);
        output_buffer_cam[length_cam] = '\0';
        int result = maixcam_parse_data((char *)output_buffer_cam);
        memset(output_buffer_cam, 0, length_cam);
    }
    
//    multiTimerStart(&mt_pid, 10, app_pid_task, NULL);
}

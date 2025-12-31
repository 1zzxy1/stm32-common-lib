/**
 ******************************************************************************
 * @file    scheduler.h
 * @brief   简单任务调度器 - 基于时间片的非抢占式调度
 * @version 1.0.0
 ******************************************************************************
 * @attention
 *
 * 轻量级任务调度器，适用于嵌入式裸机系统
 * 纯C实现，通过回调函数与硬件解耦
 *
 ******************************************************************************
 */

#ifndef _SCHEDULER_H_
#define _SCHEDULER_H_

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 配置选项 */
#ifndef SCHEDULER_MAX_TASKS
#define SCHEDULER_MAX_TASKS 16  /* 最大任务数量 */
#endif

/* 任务函数类型 */
typedef void (*scheduler_task_fn)(void);

/* 任务结构体 */
typedef struct {
    scheduler_task_fn task_func;  /* 任务函数指针 */
    uint32_t period_ms;           /* 执行周期（毫秒） */
    uint32_t last_run;            /* 上次执行时间 */
    uint8_t enabled;              /* 任务使能标志 */
} scheduler_task_t;

/* 调度器结构体 */
typedef struct {
    scheduler_task_t tasks[SCHEDULER_MAX_TASKS];  /* 任务数组 */
    uint8_t task_count;                           /* 当前任务数量 */
} scheduler_t;

/**
 * @brief 初始化调度器
 * @param sched: 调度器指针
 */
void scheduler_init(scheduler_t *sched);

/**
 * @brief 添加任务到调度器
 * @param sched: 调度器指针
 * @param task_func: 任务函数
 * @param period_ms: 执行周期（毫秒）
 * @retval 任务索引，-1表示失败
 */
int scheduler_add_task(scheduler_t *sched, scheduler_task_fn task_func, uint32_t period_ms);

/**
 * @brief 移除任务
 * @param sched: 调度器指针
 * @param task_index: 任务索引
 * @retval 0: 成功, -1: 失败
 */
int scheduler_remove_task(scheduler_t *sched, int task_index);

/**
 * @brief 使能/禁用任务
 * @param sched: 调度器指针
 * @param task_index: 任务索引
 * @param enabled: 1-使能, 0-禁用
 * @retval 0: 成功, -1: 失败
 */
int scheduler_enable_task(scheduler_t *sched, int task_index, uint8_t enabled);

/**
 * @brief 修改任务周期
 * @param sched: 调度器指针
 * @param task_index: 任务索引
 * @param period_ms: 新的执行周期（毫秒）
 * @retval 0: 成功, -1: 失败
 */
int scheduler_set_period(scheduler_t *sched, int task_index, uint32_t period_ms);

/**
 * @brief 运行调度器（需周期性调用）
 * @param sched: 调度器指针
 * @param current_time_ms: 当前系统时间（毫秒）
 */
void scheduler_run(scheduler_t *sched, uint32_t current_time_ms);

/**
 * @brief 获取任务数量
 * @param sched: 调度器指针
 * @retval 当前任务数量
 */
int scheduler_get_task_count(scheduler_t *sched);

#ifdef __cplusplus
}
#endif

#endif /* _SCHEDULER_H_ */

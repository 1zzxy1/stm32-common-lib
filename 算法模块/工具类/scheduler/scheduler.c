/**
 ******************************************************************************
 * @file    scheduler.c
 * @brief   简单任务调度器 - 基于时间片的非抢占式调度
 * @version 1.0.0
 ******************************************************************************
 */

#include "scheduler.h"

/**
 * @brief 初始化调度器
 */
void scheduler_init(scheduler_t *sched)
{
    if (sched == NULL) {
        return;
    }

    sched->task_count = 0;

    for (int i = 0; i < SCHEDULER_MAX_TASKS; i++) {
        sched->tasks[i].task_func = NULL;
        sched->tasks[i].period_ms = 0;
        sched->tasks[i].last_run = 0;
        sched->tasks[i].enabled = 0;
    }
}

/**
 * @brief 添加任务到调度器
 */
int scheduler_add_task(scheduler_t *sched, scheduler_task_fn task_func, uint32_t period_ms)
{
    if (sched == NULL || task_func == NULL) {
        return -1;
    }

    if (sched->task_count >= SCHEDULER_MAX_TASKS) {
        return -1;
    }

    int index = sched->task_count;

    sched->tasks[index].task_func = task_func;
    sched->tasks[index].period_ms = period_ms;
    sched->tasks[index].last_run = 0;
    sched->tasks[index].enabled = 1;

    sched->task_count++;

    return index;
}

/**
 * @brief 移除任务
 */
int scheduler_remove_task(scheduler_t *sched, int task_index)
{
    if (sched == NULL || task_index < 0 || task_index >= sched->task_count) {
        return -1;
    }

    /* 将后面的任务向前移动 */
    for (int i = task_index; i < sched->task_count - 1; i++) {
        sched->tasks[i] = sched->tasks[i + 1];
    }

    /* 清空最后一个位置 */
    sched->tasks[sched->task_count - 1].task_func = NULL;
    sched->tasks[sched->task_count - 1].period_ms = 0;
    sched->tasks[sched->task_count - 1].last_run = 0;
    sched->tasks[sched->task_count - 1].enabled = 0;

    sched->task_count--;

    return 0;
}

/**
 * @brief 使能/禁用任务
 */
int scheduler_enable_task(scheduler_t *sched, int task_index, uint8_t enabled)
{
    if (sched == NULL || task_index < 0 || task_index >= sched->task_count) {
        return -1;
    }

    sched->tasks[task_index].enabled = enabled ? 1 : 0;

    return 0;
}

/**
 * @brief 修改任务周期
 */
int scheduler_set_period(scheduler_t *sched, int task_index, uint32_t period_ms)
{
    if (sched == NULL || task_index < 0 || task_index >= sched->task_count) {
        return -1;
    }

    sched->tasks[task_index].period_ms = period_ms;

    return 0;
}

/**
 * @brief 运行调度器
 */
void scheduler_run(scheduler_t *sched, uint32_t current_time_ms)
{
    if (sched == NULL) {
        return;
    }

    for (int i = 0; i < sched->task_count; i++) {
        scheduler_task_t *task = &sched->tasks[i];

        /* 检查任务是否使能 */
        if (!task->enabled || task->task_func == NULL) {
            continue;
        }

        /* 检查是否到达执行时间 */
        if (current_time_ms >= task->last_run + task->period_ms) {
            /* 更新上次运行时间 */
            task->last_run = current_time_ms;

            /* 执行任务 */
            task->task_func();
        }
    }
}

/**
 * @brief 获取任务数量
 */
int scheduler_get_task_count(scheduler_t *sched)
{
    if (sched == NULL) {
        return 0;
    }

    return sched->task_count;
}

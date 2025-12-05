#ifndef SHELL_APP_H
#define SHELL_APP_H

#include <stdint.h>
#include <stdbool.h>
#include "mydefine.h"

// 配置选项
#define SHELL_MAX_COMMAND_LENGTH 64
#define SHELL_MAX_ARGS 8
#define SHELL_HISTORY_SIZE 10
#define SHELL_MAX_PATH_LEN 128
#define SHELL_MAX_LINE_LEN 80

// 命令回调函数类型
typedef int (*shell_cmd_func_t)(int argc, char *argv[]);

// 命令结构体
typedef struct {
    const char *name;          // 命令名称
    const char *help;          // 帮助信息
    shell_cmd_func_t function; // 执行函数
} shell_command_t;

// Shell状态
typedef struct {
    char cmd_buffer[SHELL_MAX_COMMAND_LENGTH];     // 命令缓冲区
    int cmd_len;                                   // 命令长度
    char history[SHELL_HISTORY_SIZE][SHELL_MAX_COMMAND_LENGTH]; // 历史命令
    int history_count;                             // 历史命令数量
    int history_index;                             // 当前历史命令索引
    int history_pos;                               // 历史浏览位置
    char current_dir[SHELL_MAX_PATH_LEN];          // 当前目录
    bool esc_seq;                                  // ESC序列标志
    bool esc_bracket;                              // ESC [ 序列标志
    lfs_t *fs;                                     // 文件系统实例指针
} shell_state_t;

// 初始化Shell
void shell_init(lfs_t *fs);

// 处理接收到的字符
void shell_process(uint8_t *buffer, uint16_t len);

// 获取当前Shell状态
shell_state_t* shell_get_state(void);

// 执行Shell命令(可供外部直接调用)
int shell_execute(const char *cmd_line);

// 输出字符串到Shell界面
void shell_print(const char *str);

// 输出格式化字符串到Shell界面
void shell_printf(const char *fmt, ...);

void shell_set_uart(void *huart);

#endif /* SHELL_APP_H */ 
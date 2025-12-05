#include "shell_app.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

// Shell状态实例
static shell_state_t shell_state;

static void *uart_handle = NULL; // 用户在初始化时设置

// 命令处理函数前向声明
static int cmd_help(int argc, char *argv[]);
static int cmd_ls(int argc, char *argv[]);
static int cmd_cd(int argc, char *argv[]);
static int cmd_pwd(int argc, char *argv[]);
static int cmd_cat(int argc, char *argv[]);
static int cmd_mkdir(int argc, char *argv[]);
static int cmd_rm(int argc, char *argv[]);
static int cmd_touch(int argc, char *argv[]);
static int cmd_mv(int argc, char *argv[]);
static int cmd_cp(int argc, char *argv[]);
static int cmd_echo(int argc, char *argv[]);
static int cmd_clear(int argc, char *argv[]);
static int cmd_write(int argc, char *argv[]);

// 可用命令列表
static const shell_command_t commands[] = {
    {"help", "Display help information", cmd_help},
    {"ls", "List directory contents", cmd_ls},
    {"cd", "Change current directory", cmd_cd},
    {"pwd", "Print working directory", cmd_pwd},
    {"cat", "Display file contents", cmd_cat},
    {"mkdir", "Create directory", cmd_mkdir},
    {"rm", "Remove file or directory", cmd_rm},
    {"touch", "Create empty file or update timestamp", cmd_touch},
    {"mv", "Move/rename file", cmd_mv},
    {"cp", "Copy file", cmd_cp},
    {"echo", "Display text", cmd_echo},
    {"clear", "Clear screen", cmd_clear},
    {"write", "Write text to file", cmd_write},
};

#define NUM_COMMANDS (sizeof(commands) / sizeof(commands[0]))

// Shell内部功能函数前向声明
static void shell_process_char(uint8_t ch);
static void shell_execute_line(void);
static void shell_handle_backspace(void);
static void shell_handle_tab(void);
static void shell_handle_escape_sequence(uint8_t ch);
static void shell_handle_history(int direction);
static void shell_add_to_history(const char *cmd);
static int shell_parse_args(char *cmd_line, char *argv[]);
static void shell_prompt(void);
static void shell_clear_line(void);
static char *shell_normalize_path(const char *path);

// 路径操作函数
static char *shell_path_join(const char *dir, const char *file);
static int shell_is_dir(const char *path);

// 初始化Shell
void shell_init(lfs_t *fs)
{
    memset(&shell_state, 0, sizeof(shell_state));
    shell_state.fs = fs;
    strcpy(shell_state.current_dir, "/");

    uart_handle = NULL; // 用户应在外部设置

    shell_printf("\r\n== LittleFS Shell v1.0 ==\r\n");
    shell_printf("Author: Microunion Studio\r\n");
    shell_prompt();
}

// 设置UART句柄
void shell_set_uart(void *huart)
{
    uart_handle = huart;
}

// 获取Shell状态
shell_state_t *shell_get_state(void)
{
    return &shell_state;
}

// 处理接收到的数据
void shell_process(uint8_t *buffer, uint16_t len)
{
    for (int i = 0; i < len; i++)
    {
        shell_process_char(buffer[i]);
    }
}

// 处理单个字符
static void shell_process_char(uint8_t ch)
{
    // 处理ESC序列 (上下左右键等)
    if (shell_state.esc_seq)
    {
        if (shell_state.esc_bracket && ch == 'A')
        {
            // 上键 - 上一条历史命令
            shell_handle_history(-1);
        }
        else if (shell_state.esc_bracket && ch == 'B')
        {
            // 下键 - 下一条历史命令
            shell_handle_history(1);
        }
        else if (ch == '[')
        {
            shell_state.esc_bracket = true;
            return;
        }
        shell_state.esc_seq = false;
        shell_state.esc_bracket = false;
        return;
    }

    // 识别ESC序列开始
    if (ch == 27)
    {
        shell_state.esc_seq = true;
        return;
    }

    // 处理回车
    if (ch == '\r' || ch == '\n')
    {
        shell_printf("\r\n");
        if (shell_state.cmd_len > 0)
        {
            shell_execute_line();
        }
        shell_prompt();
        return;
    }

    // 处理退格键
    if (ch == '\b' || ch == 127)
    {
        shell_handle_backspace();
        return;
    }

    // 处理Tab键 (命令自动补全)
    if (ch == '\t')
    {
        shell_handle_tab();
        return;
    }

    // 处理普通字符
    if (ch >= 32 && ch < 127 && shell_state.cmd_len < SHELL_MAX_COMMAND_LENGTH - 1)
    {
        shell_state.cmd_buffer[shell_state.cmd_len++] = ch;
        shell_state.cmd_buffer[shell_state.cmd_len] = '\0';
        shell_printf("%c", ch);
    }
}

// 执行命令行
static void shell_execute_line(void)
{
    // 记录历史
    shell_add_to_history(shell_state.cmd_buffer);

    // 执行命令
    shell_execute(shell_state.cmd_buffer);

    // 清空命令缓冲区
    shell_state.cmd_buffer[0] = '\0';
    shell_state.cmd_len = 0;
    shell_state.history_pos = -1;
}

// 执行命令
int shell_execute(const char *cmd_line)
{
    char cmd_copy[SHELL_MAX_COMMAND_LENGTH];
    char *argv[SHELL_MAX_ARGS];

    // 复制命令行以便解析
    strncpy(cmd_copy, cmd_line, sizeof(cmd_copy) - 1);
    cmd_copy[sizeof(cmd_copy) - 1] = '\0';

    // 解析参数
    int argc = shell_parse_args(cmd_copy, argv);
    if (argc == 0)
    {
        return 0; // 空命令
    }

    // 查找并执行命令
    for (int i = 0; i < NUM_COMMANDS; i++)
    {
        if (strcmp(argv[0], commands[i].name) == 0)
        {
            return commands[i].function(argc, argv);
        }
    }

    // 未找到命令
    shell_printf("Unknown command: %s\r\n", argv[0]);
    return -1;
}

// 解析命令行参数
static int shell_parse_args(char *cmd_line, char *argv[])
{
    int argc = 0;
    char *token = strtok(cmd_line, " \t");

    while (token != NULL && argc < SHELL_MAX_ARGS)
    {
        argv[argc++] = token;
        token = strtok(NULL, " \t");
    }

    return argc;
}

// 退格处理
static void shell_handle_backspace(void)
{
    if (shell_state.cmd_len > 0)
    {
        shell_state.cmd_len--;
        shell_state.cmd_buffer[shell_state.cmd_len] = '\0';
        shell_printf("\b \b"); // 退格，空格，再退格
    }
}

// TAB键自动补全
static void shell_handle_tab(void)
{
    char *input = shell_state.cmd_buffer;
    int input_len = shell_state.cmd_len;
    
    // 跳过输入开头的空格
    int cmd_start = 0;
    while (cmd_start < input_len && input[cmd_start] == ' ')
        cmd_start++;
    
    // 解析输入以确定当前正在输入的是命令名还是参数
    int cmd_end = cmd_start;
    while (cmd_end < input_len && input[cmd_end] != ' ')
        cmd_end++;
    
    int param_start = cmd_end;
    while (param_start < input_len && input[param_start] == ' ')
        param_start++;
    
    // 确定当前正在输入的命令名
    char cmd_name[SHELL_MAX_COMMAND_LENGTH] = {0};
    if (cmd_end > cmd_start) {
        strncpy(cmd_name, input + cmd_start, cmd_end - cmd_start);
    }
    
    // 情况1: 正在输入命令名 - 使用命令名补全
    if (param_start >= input_len) {
        const char *prefix = input + cmd_start;
        int prefix_len = input_len - cmd_start;
        int matches = 0;
        int match_index = -1;
        
        for (int i = 0; i < NUM_COMMANDS; i++) {
            if (strncmp(prefix, commands[i].name, prefix_len) == 0) {
                matches++;
                match_index = i;
            }
        }
        
        if (matches == 1) {
            // 只有一个匹配，直接补全
            strcpy(shell_state.cmd_buffer, commands[match_index].name);
            shell_state.cmd_len = strlen(shell_state.cmd_buffer);
            
            // 加一个空格以便输入参数
            shell_state.cmd_buffer[shell_state.cmd_len++] = ' ';
            shell_state.cmd_buffer[shell_state.cmd_len] = '\0';
            
            // 清除当前行并重新显示
            shell_clear_line();
            shell_printf("> %s", shell_state.cmd_buffer);
        } 
        else if (matches > 1) {
            // 多个匹配，显示所有可能的命令
            shell_printf("\r\nPossible commands:\r\n");
            for (int i = 0; i < NUM_COMMANDS; i++) {
                if (strncmp(prefix, commands[i].name, prefix_len) == 0) {
                    shell_printf("%s  ", commands[i].name);
                }
            }
            shell_printf("\r\n");
            shell_prompt();
            shell_printf("%s", shell_state.cmd_buffer);
        }
        return;
    }
    
    // 情况2: 正在输入参数 - 尝试补全文件/目录名
    
    // 检查是否为需要文件路径的命令
    int requires_path = 0;
    int dir_only = 0; // 某些命令只需要目录，如cd, mkdir
    
    if (strcmp(cmd_name, "cd") == 0 || strcmp(cmd_name, "mkdir") == 0) {
        requires_path = 1;
        dir_only = 1;
    } else if (strcmp(cmd_name, "cat") == 0 || strcmp(cmd_name, "rm") == 0 || 
               strcmp(cmd_name, "touch") == 0 || strcmp(cmd_name, "write") == 0 ||
               strcmp(cmd_name, "mv") == 0 || strcmp(cmd_name, "cp") == 0) {
        requires_path = 1;
    }
    
    if (!requires_path)
        return;
    
    // 解析当前输入的路径部分
    char partial_path[SHELL_MAX_PATH_LEN] = {0};
    strncpy(partial_path, input + param_start, input_len - param_start);
    
    // 分离目录和文件名部分
    char dir_part[SHELL_MAX_PATH_LEN] = {0};
    char file_part[SHELL_MAX_PATH_LEN] = {0};
    
    char *last_slash = strrchr(partial_path, '/');
    if (last_slash) {
        // 有目录部分
        strncpy(dir_part, partial_path, last_slash - partial_path + 1);
        strcpy(file_part, last_slash + 1);
    } else {
        // 只有文件名部分，使用当前目录
        strcpy(dir_part, "./");
        strcpy(file_part, partial_path);
    }
    
    // 规范化目录路径
    char *full_dir;
    if (dir_part[0] == 0) {
        full_dir = shell_normalize_path(".");
    } else {
        full_dir = shell_normalize_path(dir_part);
    }
    
    // 在该目录中查找匹配的文件或目录
    lfs_dir_t dir;
    if (lfs_dir_open(shell_state.fs, &dir, full_dir) != LFS_ERR_OK) {
        lfs_dir_close(shell_state.fs, &dir);
        return; // 无法打开目录
    }
    
    // 收集匹配的文件和目录
    struct {
        char name[LFS_NAME_MAX + 1];
        int is_dir;
    } matches[32];
    int match_count = 0;
    
    struct lfs_info info;
    int file_part_len = strlen(file_part);
    
    while (lfs_dir_read(shell_state.fs, &dir, &info) > 0 && match_count < 32) {
        // 跳过 . 和 ..
        if (strcmp(info.name, ".") == 0 || strcmp(info.name, "..") == 0)
            continue;
            
        // 跳过不匹配的文件/目录
        if (strncmp(file_part, info.name, file_part_len) != 0)
            continue;
            
        // 如果只需要目录且这不是目录，跳过
        if (dir_only && info.type != LFS_TYPE_DIR)
            continue;
            
        // 添加到匹配列表
        strncpy(matches[match_count].name, info.name, LFS_NAME_MAX);
        matches[match_count].name[LFS_NAME_MAX] = '\0';
        matches[match_count].is_dir = (info.type == LFS_TYPE_DIR);
        match_count++;
    }
    
    lfs_dir_close(shell_state.fs, &dir);
    
    if (match_count == 0) {
        return; // 无匹配
    } else if (match_count == 1) {
        // 只有一个匹配，直接补全
        
        // 重建完整的命令
        char new_cmd[SHELL_MAX_COMMAND_LENGTH] = {0};
        strncpy(new_cmd, input, param_start);
        
        // 添加目录部分(如果有)
        if (last_slash) {
            strncat(new_cmd, partial_path, last_slash - partial_path + 1);
        }
        
        // 添加匹配的文件名
        strcat(new_cmd, matches[0].name);
        
        // 如果是目录，添加后缀/
        if (matches[0].is_dir) {
            strcat(new_cmd, "/");
        } else {
            // 文件则添加空格便于后续输入
            strcat(new_cmd, " ");
        }
        
        // 更新命令缓冲区
        strcpy(shell_state.cmd_buffer, new_cmd);
        shell_state.cmd_len = strlen(new_cmd);
        
        // 清除当前行并重新显示
        shell_clear_line();
        shell_printf("> %s", shell_state.cmd_buffer);
    } else {
        // 多个匹配，显示所有可能的文件/目录
        shell_printf("\r\nPossible completions:\r\n");
        for (int i = 0; i < match_count; i++) {
            if (matches[i].is_dir) {
                shell_printf("[DIR]  %s/\r\n", matches[i].name);
            } else {
                shell_printf("[FILE] %s\r\n", matches[i].name);
            }
        }
        shell_printf("\r\n");
        shell_prompt();
        shell_printf("%s", shell_state.cmd_buffer);
    }
}

// 处理历史导航 (上下键)
static void shell_handle_history(int direction)
{
    if (shell_state.history_count == 0)
    {
        return; // 没有历史命令
    }

    // 更新历史导航位置
    if (shell_state.history_pos == -1)
    {
        // 首次按上键，保存当前输入
        shell_state.history_pos = 0;
    }
    else
    {
        shell_state.history_pos += direction;
    }

    // 边界检查
    if (shell_state.history_pos < 0)
    {
        shell_state.history_pos = -1; // 回到当前输入
    }
    else if (shell_state.history_pos >= shell_state.history_count)
    {
        shell_state.history_pos = shell_state.history_count - 1;
    }

    // 清除当前行
    shell_clear_line();

    if (shell_state.history_pos == -1)
    {
        // 使用临时缓存的命令
        shell_state.cmd_buffer[0] = '\0';
        shell_state.cmd_len = 0;
    }
    else
    {
        // 复制历史命令
        int idx = (shell_state.history_index - 1 - shell_state.history_pos + SHELL_HISTORY_SIZE) % SHELL_HISTORY_SIZE;
        strcpy(shell_state.cmd_buffer, shell_state.history[idx]);
        shell_state.cmd_len = strlen(shell_state.cmd_buffer);
    }

    // 显示命令
    shell_printf("> %s", shell_state.cmd_buffer);
}

// 添加命令到历史记录
static void shell_add_to_history(const char *cmd)
{
    if (strlen(cmd) == 0)
    {
        return; // 不保存空命令
    }

    // 检查是否与上一条命令相同
    if (shell_state.history_count > 0)
    {
        int last_idx = (shell_state.history_index - 1 + SHELL_HISTORY_SIZE) % SHELL_HISTORY_SIZE;
        if (strcmp(shell_state.history[last_idx], cmd) == 0)
        {
            return; // 不保存重复命令
        }
    }

    // 保存新命令
    strncpy(shell_state.history[shell_state.history_index], cmd, SHELL_MAX_COMMAND_LENGTH - 1);
    shell_state.history[shell_state.history_index][SHELL_MAX_COMMAND_LENGTH - 1] = '\0';

    shell_state.history_index = (shell_state.history_index + 1) % SHELL_HISTORY_SIZE;
    if (shell_state.history_count < SHELL_HISTORY_SIZE)
    {
        shell_state.history_count++;
    }
}

// 显示Shell提示符
static void shell_prompt(void)
{
    shell_printf("> ");
}

// 清除当前行
static void shell_clear_line(void)
{
    shell_printf("\r\033[K"); // 回到行首并清除整行
}

// 输出格式化字符串
void shell_printf(const char *fmt, ...)
{
    char buffer[128];
    va_list args;

    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    shell_print(buffer);
}

// 输出字符串
void shell_print(const char *str)
{
    if (uart_handle)
    {
        my_printf(uart_handle, "%s", str);
    }
    else
    {
        // 默认UART未设置，尝试使用直接输出
        printf("%s", str);
    }
}

// 路径处理 - 合并目录和文件名
static char *shell_path_join(const char *dir, const char *file)
{
    static char full_path[SHELL_MAX_PATH_LEN];

    // 如果file是绝对路径，则直接使用
    if (file[0] == '/')
    {
        strncpy(full_path, file, sizeof(full_path) - 1);
        full_path[sizeof(full_path) - 1] = '\0';
        return full_path;
    }

    // 拼接路径
    if (strcmp(dir, "/") == 0)
    {
        snprintf(full_path, sizeof(full_path), "/%s", file);
    }
    else
    {
        snprintf(full_path, sizeof(full_path), "%s/%s", dir, file);
    }

    return full_path;
}

// 规范化路径 (处理 . 和 .. 等)
static char *shell_normalize_path(const char *path)
{
    static char norm_path[SHELL_MAX_PATH_LEN];
    char *parts[SHELL_MAX_PATH_LEN / 2];
    int num_parts = 0;

    // 复制输入路径以便处理
    char path_copy[SHELL_MAX_PATH_LEN];
    strncpy(path_copy, path, sizeof(path_copy) - 1);
    path_copy[sizeof(path_copy) - 1] = '\0';

    // 处理绝对/相对路径
    char *curr_path;
    if (path_copy[0] == '/')
    {
        // 绝对路径
        curr_path = path_copy + 1; // 跳过开头的'/'
        parts[num_parts++] = "/";
    }
    else
    {
        // 相对路径，先复制当前路径
        curr_path = path_copy;
        if (strcmp(shell_state.current_dir, "/") == 0)
        {
            parts[num_parts++] = "/";
        }
        else
        {
            char curr_dir_copy[SHELL_MAX_PATH_LEN];
            strncpy(curr_dir_copy, shell_state.current_dir, sizeof(curr_dir_copy));

            // 将当前目录拆分为多个部分
            char *dir_part = strtok(curr_dir_copy, "/");
            while (dir_part && num_parts < SHELL_MAX_PATH_LEN / 2 - 1)
            {
                parts[num_parts++] = dir_part;
                dir_part = strtok(NULL, "/");
            }
        }
    }

    // 处理新路径部分
    char *part = strtok(curr_path, "/");
    while (part && num_parts < SHELL_MAX_PATH_LEN / 2 - 1)
    {
        if (strcmp(part, ".") == 0)
        {
            // 当前目录，忽略
        }
        else if (strcmp(part, "..") == 0)
        {
            // 上级目录，回退一级
            if (num_parts > 1 || strcmp(parts[0], "/") != 0)
            {
                num_parts--;
            }
        }
        else
        {
            // 普通目录名
            parts[num_parts++] = part;
        }
        part = strtok(NULL, "/");
    }

    // 重新构建规范化路径
    if (num_parts == 1 && strcmp(parts[0], "/") == 0)
    {
        strcpy(norm_path, "/");
    }
    else
    {
        norm_path[0] = '\0';
        for (int i = 0; i < num_parts; i++)
        {
            if (i == 0 && strcmp(parts[0], "/") == 0)
            {
                strcat(norm_path, "/");
            }
            else
            {
                strcat(norm_path, parts[i]);
                if (i < num_parts - 1)
                {
                    strcat(norm_path, "/");
                }
            }
        }
    }

    return norm_path;
}

// 检查路径是否是目录
static int shell_is_dir(const char *path)
{
    lfs_dir_t dir;
    int res = lfs_dir_open(shell_state.fs, &dir, path);
    if (res == LFS_ERR_OK)
    {
        lfs_dir_close(shell_state.fs, &dir);
        return 1;
    }
    return 0;
}

/* 命令实现 */

// help命令
static int cmd_help(int argc, char *argv[])
{
    shell_printf("Available commands:\r\n");
    for (int i = 0; i < NUM_COMMANDS; i++)
    {
        shell_printf("  %-8s - %s\r\n", commands[i].name, commands[i].help);
    }
    return 0;
}

// ls命令 - 使用树状形式显示
static int cmd_ls(int argc, char *argv[])
{
    const char *dir_path;
    int show_tree = 0; // Flag for tree display style

    // Process options
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--tree") == 0)
        {
            show_tree = 1;
            // Shift arguments
            for (int j = i; j < argc - 1; j++)
            {
                argv[j] = argv[j + 1];
            }
            argc--;
            i--;
        }
    }

    // 确定目录路径
    if (argc > 1)
    {
        dir_path = shell_normalize_path(argv[1]);
    }
    else
    {
        dir_path = shell_state.current_dir;
    }

    // 打开目录
    lfs_dir_t dir;
    int res = lfs_dir_open(shell_state.fs, &dir, dir_path);
    if (res != LFS_ERR_OK)
    {
        shell_printf("Cannot open directory '%s': Error %d\r\n", dir_path, res);
        return -1;
    }

    if (show_tree)
    {
        // Tree-style display
        shell_printf("Directory '%s' contents:\r\n", dir_path);

        // Read directory entries and store them to sort later
        struct
        {
            char name[LFS_NAME_MAX + 1];
            int is_dir;
            uint32_t size;
        } entries[64]; // Adjust size as needed

        int entry_count = 0;
        struct lfs_info info;

        while (true)
        {
            res = lfs_dir_read(shell_state.fs, &dir, &info);
            if (res <= 0)
                break; // End of directory or error

            // Skip . and ..
            if (strcmp(info.name, ".") == 0 || strcmp(info.name, "..") == 0)
            {
                continue;
            }

            if (entry_count < 64)
            {
                strncpy(entries[entry_count].name, info.name, LFS_NAME_MAX);
                entries[entry_count].name[LFS_NAME_MAX] = '\0';
                entries[entry_count].is_dir = (info.type == LFS_TYPE_DIR);
                entries[entry_count].size = info.size;
                entry_count++;
            }
        }

        // Display sorted entries in tree style
        for (int i = 0; i < entry_count; i++)
        {
            if (entries[i].is_dir)
            {
                shell_printf("+-- [DIR] %s\r\n", entries[i].name);

                // Optionally list the directory contents if necessary
                if (argc > 1 && strcmp(argv[1], "-r") == 0)
                {
                    // Create full path for this directory
                    char subdir_path[SHELL_MAX_PATH_LEN];
                    if (strcmp(dir_path, "/") == 0)
                    {
                        snprintf(subdir_path, sizeof(subdir_path), "/%s", entries[i].name);
                    }
                    else
                    {
                        snprintf(subdir_path, sizeof(subdir_path), "%s/%s", dir_path, entries[i].name);
                    }

                    // Open the subdirectory
                    lfs_dir_t subdir;
                    int subdir_res = lfs_dir_open(shell_state.fs, &subdir, subdir_path);
                    if (subdir_res == LFS_ERR_OK)
                    {
                        // Read subdirectory entries
                        struct lfs_info subinfo;
                        while (true)
                        {
                            int subres = lfs_dir_read(shell_state.fs, &subdir, &subinfo);
                            if (subres <= 0)
                                break;

                            // Skip . and ..
                            if (strcmp(subinfo.name, ".") == 0 || strcmp(subinfo.name, "..") == 0)
                            {
                                continue;
                            }

                            // Print sub-entry with indent
                            if (subinfo.type == LFS_TYPE_DIR)
                            {
                                shell_printf("    +-- [DIR] %s\r\n", subinfo.name);
                            }
                            else
                            {
                                shell_printf("    +-- [FILE] %s (%lu bytes)\r\n",
                                             subinfo.name, (unsigned long)subinfo.size);
                            }
                        }

                        lfs_dir_close(shell_state.fs, &subdir);
                    }
                }
            }
            else
            {
                shell_printf("+-- [FILE] %s (%lu bytes)\r\n", entries[i].name, (unsigned long)entries[i].size);
            }
        }

        if (entry_count == 0)
        {
            shell_printf("    (empty directory)\r\n");
        }
    }
    else
    {
        // Traditional table display
        // 读取目录项
        struct lfs_info info;
        shell_printf("Directory '%s' contents:\r\n", dir_path);
        shell_printf("  Type   Size     Name\r\n");
        shell_printf("  ----- -------- ------------------\r\n");

        while (true)
        {
            res = lfs_dir_read(shell_state.fs, &dir, &info);
            if (res < 0)
            {
                shell_printf("Failed to read directory: Error %d\r\n", res);
                break;
            }
            if (res == 0)
            {
                break; // 到达目录末尾
            }

            // 显示类型、大小和名称
            const char *type_str = (info.type == LFS_TYPE_DIR) ? "[DIR] " : "[FILE]";
            shell_printf("  %-5s %8lu %s\r\n", type_str, info.size, info.name);
        }
    }

    lfs_dir_close(shell_state.fs, &dir);
    return 0;
}

// cd命令
static int cmd_cd(int argc, char *argv[])
{
    if (argc < 2)
    {
        // 不带参数的cd命令返回根目录
        strcpy(shell_state.current_dir, "/");

        // After changing to root directory, list its contents
        shell_printf("Changed to root directory\r\n");

        // 简洁显示目录内容
        lfs_dir_t dir;
        struct lfs_info info;
        if (lfs_dir_open(shell_state.fs, &dir, "/") == LFS_ERR_OK)
        {
            shell_printf("Contents:\r\n");

            // Count entries to determine if directory is empty
            int entry_count = 0;

            while (true)
            {
                int res = lfs_dir_read(shell_state.fs, &dir, &info);
                if (res <= 0)
                    break;

                if (strcmp(info.name, ".") == 0 || strcmp(info.name, "..") == 0)
                    continue;

                entry_count++;

                if (info.type == LFS_TYPE_DIR)
                {
                    shell_printf("  [DIR]  %s\r\n", info.name);
                }
                else
                {
                    shell_printf("  [FILE] %s (%lu bytes)\r\n", info.name, (unsigned long)info.size);
                }
            }

            lfs_dir_close(shell_state.fs, &dir);

            if (entry_count == 0)
            {
                shell_printf("  (empty directory)\r\n");
            }
        }

        return 0;
    }

    // 获取规范化路径
    char *new_path = shell_normalize_path(argv[1]);

    // 检查目录是否存在
    lfs_dir_t dir;
    int res = lfs_dir_open(shell_state.fs, &dir, new_path);
    if (res != LFS_ERR_OK)
    {
        shell_printf("Cannot change to directory '%s': Error %d\r\n", new_path, res);
        return -1;
    }

    // 目录存在，更新当前目录
    strcpy(shell_state.current_dir, new_path);

    // After changing directory, display its contents in a simple format
    shell_printf("Changed to '%s'\r\n", new_path);
    shell_printf("Contents:\r\n");

    // Count entries to determine if directory is empty
    int entry_count = 0;

    // Read directory entries
    struct lfs_info info;
    while (true)
    {
        res = lfs_dir_read(shell_state.fs, &dir, &info);
        if (res <= 0)
            break;

        if (strcmp(info.name, ".") == 0 || strcmp(info.name, "..") == 0)
            continue;

        entry_count++;

        if (info.type == LFS_TYPE_DIR)
        {
            shell_printf("  [DIR]  %s\r\n", info.name);
        }
        else
        {
            shell_printf("  [FILE] %s (%lu bytes)\r\n", info.name, (unsigned long)info.size);
        }
    }

    lfs_dir_close(shell_state.fs, &dir);

    if (entry_count == 0)
    {
        shell_printf("  (empty directory)\r\n");
    }

    return 0;
}

// pwd命令
static int cmd_pwd(int argc, char *argv[])
{
    shell_printf("%s\r\n", shell_state.current_dir);
    return 0;
}

// cat命令
static int cmd_cat(int argc, char *argv[])
{
    if (argc < 2)
    {
        shell_printf("Usage: cat <filename>\r\n");
        return -1;
    }

    // 获取规范化路径
    char *file_path = shell_normalize_path(argv[1]);

    // 打开文件
    lfs_file_t file;
    int res = lfs_file_open(shell_state.fs, &file, file_path, LFS_O_RDONLY);
    if (res != LFS_ERR_OK)
    {
        shell_printf("Cannot open file '%s': Error %d\r\n", file_path, res);
        return -1;
    }

    // 读取并显示文件内容
    char buffer[SHELL_MAX_LINE_LEN + 1];
    lfs_ssize_t bytes_read;

    while ((bytes_read = lfs_file_read(shell_state.fs, &file, buffer, SHELL_MAX_LINE_LEN)) > 0)
    {
        buffer[bytes_read] = '\0';
        shell_printf("%s", buffer);
    }

    if (bytes_read < 0)
    {
        shell_printf("\r\nFile read failed: Error %d\r\n", (int)bytes_read);
    }
    else
    {
        shell_printf("\r\n");
    }

    lfs_file_close(shell_state.fs, &file);
    return (bytes_read >= 0) ? 0 : -1;
}

// mkdir命令
static int cmd_mkdir(int argc, char *argv[])
{
    if (argc < 2)
    {
        shell_printf("Usage: mkdir <directory>\r\n");
        return -1;
    }

    // 获取规范化路径
    char *dir_path = shell_normalize_path(argv[1]);

    // 创建目录
    int res = lfs_mkdir(shell_state.fs, dir_path);
    if (res != LFS_ERR_OK)
    {
        shell_printf("Cannot create directory '%s': Error %d\r\n", dir_path, res);
        return -1;
    }

    shell_printf("Directory '%s' created successfully\r\n", dir_path);
    return 0;
}

// rm命令
static int cmd_rm(int argc, char *argv[])
{
    if (argc < 2)
    {
        shell_printf("Usage: rm <filename>\r\n");
        return -1;
    }

    // 获取规范化路径
    char *path = shell_normalize_path(argv[1]);

    // 检查是否是目录
    if (shell_is_dir(path))
    {
        // 尝试删除目录
        int res = lfs_remove(shell_state.fs, path);
        if (res != LFS_ERR_OK)
        {
            shell_printf("Cannot remove directory '%s': Error %d\r\n", path, res);
            shell_printf("Note: Directory may not be empty\r\n");
            return -1;
        }
    }
    else
    {
        // 删除文件
        int res = lfs_remove(shell_state.fs, path);
        if (res != LFS_ERR_OK)
        {
            shell_printf("Cannot remove file '%s': Error %d\r\n", path, res);
            return -1;
        }
    }

    shell_printf("'%s' removed\r\n", path);
    return 0;
}

// touch命令
static int cmd_touch(int argc, char *argv[])
{
    if (argc < 2)
    {
        shell_printf("Usage: touch <filename>\r\n");
        return -1;
    }

    // 获取规范化路径
    char *file_path = shell_normalize_path(argv[1]);

    // 尝试打开文件，不存在则创建
    lfs_file_t file;
    int res = lfs_file_open(shell_state.fs, &file, file_path, LFS_O_RDWR | LFS_O_CREAT);
    if (res != LFS_ERR_OK)
    {
        shell_printf("Cannot create file '%s': Error %d\r\n", file_path, res);
        return -1;
    }

    // 关闭文件
    lfs_file_close(shell_state.fs, &file);
    shell_printf("File '%s' created/updated\r\n", file_path);
    return 0;
}

// mv命令
static int cmd_mv(int argc, char *argv[])
{
    if (argc < 3)
    {
        shell_printf("Usage: mv <source> <destination>\r\n");
        return -1;
    }

    // 获取规范化路径
    char *src_path = shell_normalize_path(argv[1]);
    char *dst_path = shell_normalize_path(argv[2]);

    // 执行重命名/移动
    int res = lfs_rename(shell_state.fs, src_path, dst_path);
    if (res != LFS_ERR_OK)
    {
        shell_printf("Cannot move/rename '%s' to '%s': Error %d\r\n", src_path, dst_path, res);
        return -1;
    }

    shell_printf("'%s' moved/renamed to '%s'\r\n", src_path, dst_path);
    return 0;
}

// cp命令
static int cmd_cp(int argc, char *argv[])
{
    if (argc < 3)
    {
        shell_printf("Usage: cp <source> <destination>\r\n");
        return -1;
    }

    // 获取规范化路径
    char *src_path = shell_normalize_path(argv[1]);
    char *dst_path = shell_normalize_path(argv[2]);

    // 打开源文件
    lfs_file_t src_file;
    int res = lfs_file_open(shell_state.fs, &src_file, src_path, LFS_O_RDONLY);
    if (res != LFS_ERR_OK)
    {
        shell_printf("Cannot open source file '%s': Error %d\r\n", src_path, res);
        return -1;
    }

    // 创建/覆盖目标文件
    lfs_file_t dst_file;
    res = lfs_file_open(shell_state.fs, &dst_file, dst_path, LFS_O_WRONLY | LFS_O_CREAT | LFS_O_TRUNC);
    if (res != LFS_ERR_OK)
    {
        shell_printf("Cannot create destination file '%s': Error %d\r\n", dst_path, res);
        lfs_file_close(shell_state.fs, &src_file);
        return -1;
    }

    // 复制数据
    char buffer[128];
    lfs_ssize_t bytes_read, bytes_written;
    lfs_ssize_t total_bytes = 0;

    while ((bytes_read = lfs_file_read(shell_state.fs, &src_file, buffer, sizeof(buffer))) > 0)
    {
        bytes_written = lfs_file_write(shell_state.fs, &dst_file, buffer, bytes_read);
        if (bytes_written != bytes_read)
        {
            shell_printf("Error writing to destination file: Requested %ld bytes, wrote %ld bytes\r\n",
                         (long)bytes_read, (long)bytes_written);
            lfs_file_close(shell_state.fs, &src_file);
            lfs_file_close(shell_state.fs, &dst_file);
            return -1;
        }
        total_bytes += bytes_written;
    }

    // 关闭文件
    lfs_file_close(shell_state.fs, &src_file);
    lfs_file_close(shell_state.fs, &dst_file);

    if (bytes_read < 0)
    {
        shell_printf("Error reading source file: %d\r\n", (int)bytes_read);
        return -1;
    }

    shell_printf("Copied '%s' to '%s', %ld bytes total\r\n", src_path, dst_path, (long)total_bytes);
    return 0;
}

// echo命令
static int cmd_echo(int argc, char *argv[])
{
    for (int i = 1; i < argc; i++)
    {
        shell_printf("%s", argv[i]);
        if (i < argc - 1)
        {
            shell_printf(" ");
        }
    }
    shell_printf("\r\n");
    return 0;
}

// clear命令
static int cmd_clear(int argc, char *argv[])
{
    shell_printf("\033[2J\033[H"); // 清屏并将光标移至左上角
    return 0;
}

// write命令 - 向文件写入文本内容
static int cmd_write(int argc, char *argv[])
{
    if (argc < 3)
    {
        shell_printf("Usage: write <filename> <text>\r\n");
        shell_printf("Example: write test.txt Hello world\r\n");
        return -1;
    }

    // 获取规范化路径
    char *file_path = shell_normalize_path(argv[1]);

    // 打开文件用于写入(覆盖已有内容)
    lfs_file_t file;
    int res = lfs_file_open(shell_state.fs, &file, file_path, LFS_O_WRONLY | LFS_O_CREAT | LFS_O_TRUNC);
    if (res != LFS_ERR_OK)
    {
        shell_printf("Cannot open file '%s' for writing: Error %d\r\n", file_path, res);
        return -1;
    }

    // 将参数中的文本写入文件
    for (int i = 2; i < argc; i++)
    {
        // 写入参数
        lfs_ssize_t w_sz = lfs_file_write(shell_state.fs, &file, argv[i], strlen(argv[i]));
        if (w_sz < 0)
        {
            shell_printf("Failed to write to file: Error %d\r\n", (int)w_sz);
            lfs_file_close(shell_state.fs, &file);
            return -1;
        }

        // 如果不是最后一个参数，添加空格
        if (i < argc - 1)
        {
            lfs_file_write(shell_state.fs, &file, " ", 1);
        }
    }

    lfs_file_close(shell_state.fs, &file);
    shell_printf("Data written to '%s'\r\n", file_path);
    return 0;
}
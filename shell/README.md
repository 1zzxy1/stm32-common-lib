# Shell 命令行接口

> 🔧 **LittleFS专用** - 需要配合LittleFS文件系统使用

基于串口的Linux风格命令行Shell，支持文件系统操作、命令历史、TAB补全等功能。

## 特性

- Linux风格命令（ls、cd、cat、mkdir等）
- 命令历史记录（上下键切换）
- TAB自动补全（命令和路径）
- 路径规范化（支持. 和 ..）
- 文件系统浏览和管理
- 树状目录显示

## 强依赖

**必须**配合以下模块使用：
- ✅ **lfs** - LittleFS文件系统
- ✅ **spi_flash** - Flash存储驱动
- ✅ **STM32 HAL** - 串口通信

**修改点**（在shell_app.h中）:
```c
#include "mydefine.h"  // 需要包含lfs.h
```

## 支持的命令

| 命令 | 说明 | 示例 |
|------|------|------|
| `help` | 显示帮助信息 | `help` |
| `ls` | 列出目录内容 | `ls`, `ls /data`, `ls -t` |
| `cd` | 切换目录 | `cd /data`, `cd ..` |
| `pwd` | 显示当前目录 | `pwd` |
| `cat` | 显示文件内容 | `cat test.txt` |
| `mkdir` | 创建目录 | `mkdir mydir` |
| `rm` | 删除文件/目录 | `rm test.txt` |
| `touch` | 创建空文件 | `touch new.txt` |
| `mv` | 移动/重命名 | `mv old.txt new.txt` |
| `cp` | 复制文件 | `cp src.txt dst.txt` |
| `echo` | 显示文本 | `echo Hello World` |
| `clear` | 清屏 | `clear` |
| `write` | 写入文件 | `write test.txt Hello` |

## 使用方法

### 1. 初始化

```c
#include "shell_app.h"
#include "lfs.h"

extern lfs_t lfs;  // LittleFS实例
extern UART_HandleTypeDef huart3;  // 串口句柄

void Shell_Init(void)
{
    // 设置UART句柄
    shell_set_uart(&huart3);

    // 初始化Shell
    shell_init(&lfs);
}
```

### 2. 接收数据处理

```c
// 在串口接收回调中
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart == &huart3) {
        uint8_t rx_data;
        HAL_UART_Receive(&huart3, &rx_data, 1, HAL_MAX_DELAY);

        // 处理接收到的字符
        shell_process(&rx_data, 1);
    }
}
```

### 3. 串口输出配置

需要实现`my_printf`函数（在shell_app.c中使用）:

```c
void my_printf(void *huart, const char *fmt, ...)
{
    char buffer[128];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    HAL_UART_Transmit((UART_HandleTypeDef*)huart,
                      (uint8_t*)buffer,
                      strlen(buffer),
                      HAL_MAX_DELAY);
}
```

## 配置选项

在`shell_app.h`中修改：

```c
#define SHELL_MAX_COMMAND_LENGTH 64   // 最大命令长度
#define SHELL_MAX_ARGS 8              // 最大参数数量
#define SHELL_HISTORY_SIZE 10         // 历史记录数量
#define SHELL_MAX_PATH_LEN 128        // 最大路径长度
#define SHELL_MAX_LINE_LEN 80         // 最大行长度
```

## 交互特性

### 命令历史
- 按**上键**：浏览上一条命令
- 按**下键**：浏览下一条命令
- 自动去重：不记录连续重复命令

### TAB补全
- 命令补全：输入部分命令名按TAB补全
- 路径补全：输入部分路径按TAB显示匹配项
- 智能过滤：根据命令类型过滤（如cd只显示目录）

### 路径处理
- 支持相对路径：`cd data`
- 支持绝对路径：`cd /data`
- 支持当前目录：`cd .`
- 支持上级目录：`cd ..`

## API概览

| 函数 | 说明 |
|------|------|
| `shell_init()` | 初始化Shell |
| `shell_set_uart()` | 设置串口句柄 |
| `shell_process()` | 处理接收数据 |
| `shell_execute()` | 执行命令 |
| `shell_printf()` | 格式化输出 |
| `shell_print()` | 字符串输出 |
| `shell_get_state()` | 获取Shell状态 |

## 示例会话

```
== LittleFS Shell v1.0 ==
Author: Microunion Studio
> ls
Directory '/' contents:
  Type   Size     Name
  ----- -------- ------------------
  [DIR]        0 data
  [FILE]      14 test.txt
> cd data
Changed to '/data'
Contents:
  [FILE] config.ini (128 bytes)
> cat ../test.txt
Hello LittleFS
> write myfile.txt Hello World
Data written to '/data/myfile.txt'
> ls -t
Directory '/data' contents:
+-- [FILE] config.ini (128 bytes)
+-- [FILE] myfile.txt (11 bytes)
```

## 注意事项

1. **必须**先初始化LittleFS文件系统
2. **必须**实现`my_printf`函数
3. 串口配置建议：115200波特率，8N1
4. 命令不支持引号（空格会分隔参数）
5. 文件名不支持空格
6. 删除目录前需确保目录为空

## 内存占用

- 代码: ~35KB
- RAM: ~1KB（命令缓冲 + 历史记录）
- Stack: ~1KB（路径处理）

## 扩展开发

### 添加自定义命令

在`shell_app.c`中：

```c
// 1. 添加命令处理函数
static int cmd_mycommand(int argc, char *argv[])
{
    shell_printf("My custom command!\r\n");
    return 0;
}

// 2. 注册到命令表
static const shell_command_t commands[] = {
    // ... 其他命令
    {"mycommand", "My custom command", cmd_mycommand},
};
```

## 限制

- 不支持管道和重定向
- 不支持通配符（*、?）
- 不支持后台执行
- 不支持脚本文件
- 单线程执行

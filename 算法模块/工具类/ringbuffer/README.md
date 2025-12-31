# 环形缓冲区 (Ring Buffer)

基于 RT-Thread 的环形缓冲区实现，适用于串口收发、数据流处理等场景。

## 特性

- 使用镜像位技术区分满/空状态
- 支持批量读写和单字节操作
- 支持强制写入（覆盖旧数据）
- 零拷贝 peek 操作
- 纯C实现，无硬件依赖

## 许可证

Apache-2.0 (来自 RT-Thread)

## 使用方法

### 1. 初始化

```c
#include "ringbuffer.h"

// 定义缓冲区和结构体
uint8_t buffer[128];
struct rb_ringbuffer rb;

// 初始化
rb_ringbuffer_init(&rb, buffer, sizeof(buffer));
```

### 2. 写入数据

```c
// 写入多个字节（空间不足时丢弃）
uint8_t data[] = "Hello";
rb_size_t written = rb_ringbuffer_put(&rb, data, sizeof(data));

// 写入单个字节
rb_ringbuffer_putchar(&rb, 'A');

// 强制写入（覆盖旧数据）
rb_ringbuffer_put_force(&rb, data, sizeof(data));
rb_ringbuffer_putchar_force(&rb, 'B');
```

### 3. 读取数据

```c
// 读取多个字节
uint8_t recv[64];
rb_size_t read_len = rb_ringbuffer_get(&rb, recv, sizeof(recv));

// 读取单个字节
uint8_t ch;
if (rb_ringbuffer_getchar(&rb, &ch)) {
    // 读取成功
}
```

### 4. 状态查询

```c
// 获取数据长度
rb_size_t len = rb_ringbuffer_data_len(&rb);

// 获取剩余空间
rb_size_t space = rb_ringbuffer_space_len(&rb);

// 获取状态
enum rb_ringbuffer_state state = rb_ringbuffer_status(&rb);
// RB_RINGBUFFER_EMPTY  - 空
// RB_RINGBUFFER_FULL   - 满
// RB_RINGBUFFER_HALFFULL - 有数据

// 重置缓冲区
rb_ringbuffer_reset(&rb);
```

### 5. 串口接收示例

```c
struct rb_ringbuffer uart_rb;
uint8_t uart_buffer[256];

void UART_Init(void)
{
    rb_ringbuffer_init(&uart_rb, uart_buffer, sizeof(uart_buffer));
}

// 中断服务函数
void UART_IRQHandler(void)
{
    uint8_t ch = UART_ReadByte();
    rb_ringbuffer_putchar_force(&uart_rb, ch);
}

// 主循环处理
void process_uart_data(void)
{
    uint8_t ch;
    while (rb_ringbuffer_getchar(&uart_rb, &ch)) {
        // 处理接收到的数据
    }
}
```

## API 说明

| 函数 | 说明 |
|------|------|
| `rb_ringbuffer_init()` | 初始化环形缓冲区 |
| `rb_ringbuffer_reset()` | 重置缓冲区 |
| `rb_ringbuffer_status()` | 获取缓冲区状态 |
| `rb_ringbuffer_put()` | 写入数据 |
| `rb_ringbuffer_put_force()` | 强制写入数据 |
| `rb_ringbuffer_putchar()` | 写入单字节 |
| `rb_ringbuffer_putchar_force()` | 强制写入单字节 |
| `rb_ringbuffer_get()` | 读取数据 |
| `rb_ringbuffer_getchar()` | 读取单字节 |
| `rb_ringbuffer_peek()` | 零拷贝读取 |
| `rb_ringbuffer_data_len()` | 获取数据长度 |
| `rb_ringbuffer_space_len()` | 获取剩余空间 |
| `rb_ringbuffer_get_size()` | 获取缓冲区大小 |

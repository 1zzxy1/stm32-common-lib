# Bit Array 位数组库

高效的位数组操作库，纯头文件实现。

## 特性

- **Header-only** - 仅包含头文件即可使用
- 支持基本位操作：get/set/clear/toggle
- 支持批量操作：clear_all/set_all/toggle_all
- 支持逻辑运算：AND/OR/XOR/NOT
- 支持统计：popcount（汉明权重）
- 支持32位和64位系统
- 纯C实现，无外部依赖

## 使用方法

### 1. 定义位数组

```c
#include "bit_array.h"

// 定义一个64位的位数组
BIT_ARRAY_DEFINE(flags, 64);

// 或手动定义
bit_array_t my_flags[BIT_ARRAY_BITMAP_SIZE(100)];  // 100位
```

### 2. 基本操作

```c
// 设置位
bit_array_set(flags, 5);      // 设置第5位为1

// 清除位
bit_array_clear(flags, 5);    // 清除第5位

// 获取位
int val = bit_array_get(flags, 5);  // 获取第5位的值

// 切换位
bit_array_toggle(flags, 5);   // 切换第5位

// 赋值
bit_array_assign(flags, 5, 1);  // 设置第5位为指定值
```

### 3. 批量操作

```c
// 清除所有位
bit_array_clear_all(flags, 64);

// 设置所有位
bit_array_set_all(flags, 64);

// 切换所有位
bit_array_toggle_all(flags, 64);
```

### 4. 逻辑运算

```c
BIT_ARRAY_DEFINE(a, 32);
BIT_ARRAY_DEFINE(b, 32);
BIT_ARRAY_DEFINE(result, 32);

// 按位与
bit_array_and(result, a, b, 32);

// 按位或
bit_array_or(result, a, b, 32);

// 按位异或
bit_array_xor(result, a, b, 32);

// 按位取反
bit_array_not(result, a, 32);
```

### 5. 统计和比较

```c
// 统计置位数量
int count = bit_array_num_bits_set(flags, 64);

// 统计清零数量
int cleared = bit_array_num_bits_cleared(flags, 64);

// 比较两个位数组
int cmp = bit_array_cmp(a, b, 32);
```

### 6. 复制和转换

```c
// 复制
bit_array_copy_all(dst, src, 64);

// 转换为字符串 (需要 num_bits+1 的缓冲区)
char str[65];
bit_array_to_str(flags, 64, str);
printf("bits: %s\n", str);
```

## 配置选项

```c
// 在包含头文件前定义，启用64位模式
#define BIT_ARRAY_CONFIG_64
#include "bit_array.h"
```

## API 概览

| 函数 | 说明 |
|------|------|
| `bit_array_get()` | 获取指定位的值 |
| `bit_array_set()` | 设置指定位为1 |
| `bit_array_clear()` | 清除指定位 |
| `bit_array_toggle()` | 切换指定位 |
| `bit_array_assign()` | 设置指定位为给定值 |
| `bit_array_clear_all()` | 清除所有位 |
| `bit_array_set_all()` | 设置所有位 |
| `bit_array_toggle_all()` | 切换所有位 |
| `bit_array_and()` | 按位与 |
| `bit_array_or()` | 按位或 |
| `bit_array_xor()` | 按位异或 |
| `bit_array_not()` | 按位取反 |
| `bit_array_num_bits_set()` | 统计置位数 |
| `bit_array_cmp()` | 比较两个位数组 |
| `bit_array_copy_all()` | 复制位数组 |
| `bit_array_to_str()` | 转换为字符串 |

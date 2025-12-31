# Makefile 使用示例

> ✅ **通用模块** - 适用于Linux/PC平台

## 功能特性

- Makefile基础使用示例
- 多文件编译
- 依赖关系管理
- 增量编译
- 清理规则

## 文件说明

```
makefile_example/
├── hello.c        # 主程序
├── add.c          # 加法函数
├── makefile       # Makefile文件
└── compiler.sh    # 编译脚本
```

## 使用场景

✅ **适用于**：
- Makefile学习入门
- C/C++项目构建
- 自动化编译
- 依赖管理
- Linux开发

## Makefile内容

```makefile
hello: add.o hello.o
	gcc add.o hello.o -o hello

add.o: add.c
	gcc -c add.c

hello.o: hello.c
	gcc -c hello.c
```

## 使用方法

### 编译程序
```bash
make
```

### 清理编译文件
```bash
# 添加清理规则到makefile
clean:
	rm -f *.o hello

# 执行清理
make clean
```

### 增量编译
```bash
# 修改add.c后，只会重新编译add.c
make
```

## Makefile语法说明

### 基本结构
```makefile
目标: 依赖文件
	命令（必须用Tab缩���）
```

### 变量使用
```makefile
CC = gcc
CFLAGS = -Wall -O2

hello: hello.o
	$(CC) $(CFLAGS) hello.o -o hello
```

### 自动变量
- `$@`：目标文件名
- `$<`：第一个依赖文件
- `$^`：所有依赖文件

### 通配符
```makefile
# 编译所有.c文件
%.o: %.c
	gcc -c $< -o $@
```

## 常用Makefile模板

```makefile
# 编译器
CC = gcc
CFLAGS = -Wall -O2

# 目标文件
TARGET = hello
OBJS = main.o func1.o func2.o

# 默认目标
all: $(TARGET)

# 链接
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET)

# 编译
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# 清理
clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
```

## 高级用法

### 条件编译
```makefile
ifdef DEBUG
CFLAGS += -g
else
CFLAGS += -O2
endif
```

### 包含其他文件
```makefile
include config.mk
```

### 函数使用
```makefile
SRCS = $(wildcard *.c)
OBJS = $(patsubst %.c, %.o, $(SRCS))
```

## 注意事项

1. **Tab缩进**：命令行必须用Tab键缩进，不能用空格
2. **依赖关系**：正确设置文件依赖关系
3. **伪目标**：使用.PHONY声明伪目标（clean、all等）
4. **路径问题**：注意相对路径和绝对路径
5. **并行编译**：使用`make -j4`进行多核并行编译

## 常用命令

```bash
make              # 编译（使用默认目标）
make clean        # 清理
make all          # 编译所有目标
make -j4          # 4线程并行编译
make -n           # 显示命令但不执行（调试用）
make -B           # 强制重新编译所有文件
```

## 依赖项

- GNU Make
- GCC编译器
- Linux/Unix系统

## 来源

网友那拿的（Linux开发基础示例）

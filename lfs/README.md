# LittleFS 文件系统

> 📦 **第三方库** - 轻量级嵌入式文件系统

LittleFS是一个专为微控制器设计的小型失效安全文件系统。

## 特性

- 掉电安全：保证文件系统完整性
- 磨损均衡：延长Flash寿命
- 动态块分配
- 小RAM占用（约700字节）
- 目录树结构
- 支持文件元数据

## 文件说明

```
lfs/
├── lfs.c          # LittleFS核心实现
├── lfs.h          # LittleFS头文件
├── lfs_util.c     # 工具函数
├── lfs_util.h     # 工具函数头文件
├── lfs_port.c     # 移植层（硬件接口）
├── lfs_port.h     # 移植层头文件
└── README.md
```

## 依赖

- **硬件**: SPI Flash芯片（如GD25Qxx）
- **驱动**: spi_flash模块（gd25qxx）

## 移植要求

**必须实现的硬件接口**（在lfs_port.c中）:

```c
// 读取块
int lfs_port_read(const struct lfs_config *c, lfs_block_t block,
                  lfs_off_t off, void *buffer, lfs_size_t size);

// 写入块
int lfs_port_prog(const struct lfs_config *c, lfs_block_t block,
                  lfs_off_t off, const void *buffer, lfs_size_t size);

// 擦除块
int lfs_port_erase(const struct lfs_config *c, lfs_block_t block);

// 同步（可选）
int lfs_port_sync(const struct lfs_config *c);
```

## 使用方法

### 1. 配置文件系统

修改`lfs_port.c`中的配置：

```c
// 块大小配置（根据Flash扇区大小）
#define LFS_BLOCK_SIZE 4096     // 4KB扇区
#define LFS_BLOCK_COUNT 512     // Flash总块数
#define LFS_READ_SIZE 256
#define LFS_PROG_SIZE 256
#define LFS_CACHE_SIZE 256
#define LFS_LOOKAHEAD_SIZE 16
```

### 2. 初始化

```c
#include "lfs.h"
#include "lfs_port.h"

lfs_t lfs;
struct lfs_config cfg;

void FileSystem_Init(void)
{
    // 获取配置
    lfs_port_get_default_config(&cfg);

    // 挂载文件系统
    int err = lfs_mount(&lfs, &cfg);

    // 如果挂载失败，格式化
    if (err) {
        lfs_format(&lfs, &cfg);
        lfs_mount(&lfs, &cfg);
    }
}
```

### 3. 文件操作

```c
// 写入文件
lfs_file_t file;
lfs_file_open(&lfs, &file, "test.txt", LFS_O_WRONLY | LFS_O_CREAT);
lfs_file_write(&lfs, &file, "Hello LittleFS", 14);
lfs_file_close(&lfs, &file);

// 读取文件
char buffer[64];
lfs_file_open(&lfs, &file, "test.txt", LFS_O_RDONLY);
lfs_file_read(&lfs, &file, buffer, sizeof(buffer));
lfs_file_close(&lfs, &file);

// 删除文件
lfs_remove(&lfs, "test.txt");
```

### 4. 目录操作

```c
// 创建目录
lfs_mkdir(&lfs, "mydir");

// 遍历目录
lfs_dir_t dir;
struct lfs_info info;
lfs_dir_open(&lfs, &dir, "/");

while (lfs_dir_read(&lfs, &dir, &info) > 0) {
    printf("%s %s %ld bytes\r\n",
           info.type == LFS_TYPE_DIR ? "[DIR]" : "[FILE]",
           info.name, info.size);
}

lfs_dir_close(&lfs, &dir);
```

## API概览

### 文件操作
- `lfs_file_open()` - 打开文件
- `lfs_file_close()` - 关闭文件
- `lfs_file_read()` - 读取文件
- `lfs_file_write()` - 写入文件
- `lfs_file_seek()` - 移动文件指针
- `lfs_file_size()` - 获取文件大小

### 目录操作
- `lfs_mkdir()` - 创建目录
- `lfs_dir_open()` - 打开目录
- `lfs_dir_read()` - 读取目录项
- `lfs_dir_close()` - 关闭目录
- `lfs_remove()` - 删除文件/目录
- `lfs_rename()` - 重命名

### 文件系统操作
- `lfs_format()` - 格式化文件系统
- `lfs_mount()` - 挂载文件系统
- `lfs_unmount()` - 卸载文件系统

## 内存占用

- 代码: ~10KB
- RAM: ~700字节（基础）+ 缓存大小
- Stack: ~500字节

## 配置参数

| 参数 | 说明 | 典型值 |
|------|------|--------|
| `read_size` | 最小读取单元 | 256 |
| `prog_size` | 最小编程单元 | 256 |
| `block_size` | 擦除块大小 | 4096 |
| `block_count` | 总块数 | 根据Flash容量 |
| `cache_size` | 缓存大小 | 256 |
| `lookahead_size` | 预读缓存 | 16 |

## 注意事项

1. 块大小必须与Flash扇区大小匹配
2. 需要实现硬件读写擦除接口
3. 掉电安全但会牺牲一定性能
4. 不支持多线程（需外部加锁）
5. 文件名长度限制（默认255字符）

## 参考资源

- [LittleFS GitHub](https://github.com/littlefs-project/littlefs)
- [LittleFS设计文档](https://github.com/littlefs-project/littlefs/blob/master/DESIGN.md)

# Linux TCP 网络编程示例

> ✅ **通用模块** - 适用于Linux/PC平台

## 功能特性

- 完整的TCP服务器和客户端实现
- Qt界面支持
- 中文消息支持
- 兼容telnet客户端
- 可扩展架构

## 目录结构

```
linux_tcp/
├── server/          # TCP服务器（Qt）
│   ├── main.cpp
│   ├── mainwindow.cpp
│   ├── mainwindow.h
│   ├── mainwindow.ui
│   └── server.pro
└── client/          # TCP客户端（Qt）
    ├── main.cpp
    ├── mainwindow.cpp
    ├── mainwindow.h
    ├── mainwindow.ui
    └── client.pro
```

## 使用场景

✅ **适用于**：
- Linux网络编程学习
- TCP通信调试
- Qt网络应用开发
- 物联网数据传输
- 远程控制系统

## 编译运行

### 服务器

```bash
cd server
qmake server.pro
make
./server
```

### 客户端

```bash
cd client
qmake client.pro
make
./client
```

### 使用telnet测试

```bash
# 连接到服务器（假设监听8888端口）
telnet localhost 8888
```

## 主要功能

### 服务器端
- 监听指定端口
- 接受多个客户端连接
- 接收客户端消息
- 广播消息到所有客户端
- 显示连接状态

### 客户端
- 连接到服务器
- 发送消息
- 接收服务器消息
- 断开重连

## 技术要点

- Qt Network模块（QTcpServer、QTcpSocket）
- 中文编码处理（UTF-8）
- 多客户端管理
- 非阻塞IO
- 信号槽机制

## 依赖项

- Qt5 或 Qt6
- Linux系统（或Windows with MinGW）
- C++编译器

## 注意事项

1. **端口配置**：修改端口号在代码中搜索端口定义
2. **防火墙**：确保端口未被防火墙阻挡
3. **编码**：统一使用UTF-8编码避免乱码
4. **并发**：大量连接时注意性能优化

## 来源

网友那拿的（Linux嵌入式课程示例）

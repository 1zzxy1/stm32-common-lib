# Linux 线程编程示例

> ✅ **通用模块** - 适用于Linux/PC平台

## 功能特性

- Qt线程使用示例
- QThread类封装
- 线程间通信（信号槽）
- UI线程与工作线程分离
- 线程生命周期管理

## 文件说明

```
linux_thread/
├── main.cpp           # 主程序入口
├── mainwindow.cpp/h   # 主窗口类
├── mainwindow.ui      # UI界面文件
├── mythread.cpp/h     # 自定义线程类
└── untitled47.pro     # Qt工程文件
```

## 使用场景

✅ **适用于**：
- Qt多线程开发学习
- 耗时任务后台处理
- 避免UI阻塞
- 并发编程练习
- 实时数据处理

## 编译运行

```bash
cd linux_thread
qmake untitled47.pro
make
./untitled47
```

## 技术要点

### 线程类设计
```cpp
class MyThread : public QThread
{
    Q_OBJECT
public:
    void run() override;  // 重写run函数
signals:
    void progress(int);   // 进度信号
};
```

### 线程使用
```cpp
// 创建线程
MyThread *thread = new MyThread();

// 连接信号
connect(thread, &MyThread::progress,
        this, &MainWindow::onProgress);

// 启动线程
thread->start();

// 等待线程结束
thread->wait();
```

## 核心概念

1. **QThread类**：Qt的线程封装
2. **run()函数**：线程执行体
3. **信号槽机制**：跨线程通信
4. **moveToThread()**：对象移到线程
5. **线程安全**：避免数据竞争

## 注意事项

1. **UI操作**：只能在主线程中操作UI
2. **信号槽**：跨线程使用Qt::QueuedConnection
3. **资源释放**：线程结束后及时释放
4. **死锁避免**：注意互斥锁的使用顺序
5. **线程数量**：不宜创建过多线程

## 依赖项

- Qt5 或 Qt6
- Linux系统（或Windows）
- C++11及以上

## 来源

网友那拿的（Linux嵌入式课程示例）

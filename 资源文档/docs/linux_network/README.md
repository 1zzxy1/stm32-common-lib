# Linux网络管理技术 文档资料

> 📚 **学习资料** - Linux网络服务配置与管理文档集合

## 目录结构

```
docs/linux_network/
├── 教材/                    # PDF教材
├── 配置文件/                 # 配置示例和教程
├── 笔记/                    # 学习笔记
└── README.md               # 本文档
```

## 文档列表

### 📘 教材（教材/）

| 文件名 | 说明 |
|-------|------|
| `教材--网络服务搭建、配置与管理大全：Linux版.pdf` | 主教材：Linux网络服务全面讲解 |
| `CentOS6.5下Samba服务器的安装与配置.pdf` | Samba文件共享服务配置 |
| `SSH端口转发详解.pdf` | SSH隧道和端口转发技术 |

### 🔧 配置文件（配置文件/）

#### linux下的磁盘分区，swap设置/

| 文件名 | 说明 |
|-------|------|
| `ubuntu设置swap.docx` | Ubuntu系统Swap分区设置教程 |
| `为虚拟机添加虚拟硬盘.docx` | VMware虚拟机添加硬盘步骤 |

### 📝 笔记文档（笔记/）

| 文件名 | 说明 |
|-------|------|
| `linux简介.docx` | Linux系统基础介绍 |
| `网络通信.doc` | Linux网络通信基础 |
| `补充部分.docx` | 补充学习材料 |

## 使用场景

✅ **适用于**：
- Linux系统管理员学习
- 网络服务搭建和配置
- 服务器运维管理
- 虚拟机环境配置
- 文件共享服务部署

## 技术栈覆盖

### 网络服务
- **Samba** - Windows/Linux文件共享
- **SSH** - 远程登录和端口转发
- **DNS** - 域名解析服务
- **DHCP** - 动态主机配置
- **HTTP** - Web服务器（Apache/Nginx）
- **FTP** - 文件传输服务

### 系统管理
- 磁盘分区管理
- Swap交换空间配置
- 虚拟机硬盘管理
- 网络配置和调试

## 学习路径建议

1. **Linux基础** → 先阅读"linux简介"
2. **网络基础** → 学习"网络通信"文档
3. **服务搭建** → 参考主教材配置各类网络服务
4. **实战练习** →
   - 配置Samba实现Windows/Linux文件共享
   - 配置SSH实现远程管理和端口转发
   - 搭建Web服务器
5. **系统优化** → 学习磁盘分区和Swap配置

## 常用服务端口

| 服务 | 默认端口 | 协议 |
|-----|---------|------|
| SSH | 22 | TCP |
| FTP | 20, 21 | TCP |
| HTTP | 80 | TCP |
| HTTPS | 443 | TCP |
| Samba | 139, 445 | TCP |
| DNS | 53 | UDP/TCP |

## 注意事项

1. **防火墙配置**：配置服务后记得开放对应端口
2. **权限管理**：注意文件和目录权限设置
3. **安全性**：
   - 修改SSH默认端口
   - 禁用root远程登录
   - 使用密钥认证代替密码
4. **系统版本**：注意文档中的系统版本（CentOS/Ubuntu）
5. **备份配置**：修改配置前先备份原文件

## 实用命令参考

```bash
# 网络配置
ifconfig / ip addr                # 查看网络接口
ping <host>                       # 测试网络连通性
netstat -tuln                     # 查看监听端口
ss -tuln                          # 查看套接字状态

# 服务管理
systemctl start <service>         # 启动服务
systemctl enable <service>        # 开机自启
systemctl status <service>        # 查看服务状态

# 防火墙（firewalld）
firewall-cmd --list-all           # 查看防火墙规则
firewall-cmd --add-port=80/tcp --permanent  # 开放端口
firewall-cmd --reload             # 重载配置

# 磁盘管理
fdisk -l                          # 查看磁盘分区
df -h                             # 查看磁盘使用情况
free -h                           # 查看内存和Swap
swapon -s                         # 查看Swap信息
```

## 来源

网友那拿的（Linux网络管理课程教材和笔记）

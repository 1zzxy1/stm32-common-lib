# 开发工具配置文件

> 🔧 **配置文件** - AI辅助开发工具配置集合

## 目录结构

```
configs/dev_tools/
├── .claude/                # Claude Code配置
├── .mcp.json              # MCP服务器配置
├── .serena/               # Serena配置
├── .spec-workflow/        # Spec Workflow配置
└── README.md              # 本文档
```

## 配置文件说明

### 🤖 Claude Code（.claude/）

Claude Code是Anthropic推出的AI编程助手命令行工具。

**配置文件**：
- `settings.local.json` - 本地设置（API密钥、模型选择等）

**使用场景**：
- AI辅助编码
- 代码审查和优化
- 文档生成
- 问题诊断和调试

**官网**：https://claude.com/claude-code

### 🔌 MCP服务器（.mcp.json）

Model Context Protocol (MCP) 服务器配置文件，用于扩展Claude Code功能。

**配置内容**：
- MCP服务器列表
- 服务器连接参数
- 工具权限配置

**使用场景**：
- 扩展Claude Code工具集
- 集成外部服务（数据库、API等）
- 自定义开发工作流

### 🧠 Serena（.serena/）

Serena是AI辅助项目管理工具。

**配置文件**：
- `project.yml` - 项目配置

**使用场景**：
- 项目需求管理
- 任务分解和追踪
- 开发计划制定

### 📋 Spec Workflow（.spec-workflow/）

规格说明工作流配置，用于管理软件开发的需求和设计文档。

**目录结构**：
```
.spec-workflow/
├── templates/           # 文档模板
│   ├── design-template.md
│   ├── product-template.md
│   ├── requirements-template.md
│   ├── structure-template.md
│   ├── tasks-template.md
│   └── tech-template.md
└── user-templates/      # 用户自定义模板
```

**使用场景**：
- 需求文档编写
- 技术设计文档
- 任务分解规划
- 产品规格说明

## 使用方法

### 快速开始

1. **复制配置文件到项目根目录**
   ```bash
   cp -r configs/dev_tools/.claude ./
   cp configs/dev_tools/.mcp.json ./
   cp -r configs/dev_tools/.serena ./
   cp -r configs/dev_tools/.spec-workflow ./
   ```

2. **修改配置**
   - 编辑`.claude/settings.local.json`，填入API密钥
   - 编辑`.mcp.json`，配置需要的MCP服务器
   - 根据项目需求修改其他配置

3. **启动工具**
   ```bash
   # Claude Code
   claude-code

   # 或使用VS Code扩展
   ```

### Claude Code 配置示例

```json
{
  "apiKey": "your-api-key-here",
  "model": "claude-sonnet-4-5",
  "maxTokens": 200000,
  "temperature": 0.7
}
```

### MCP 配置示例

```json
{
  "mcpServers": {
    "context7": {
      "command": "npx",
      "args": ["-y", "@context7/mcp-server"]
    },
    "playwright": {
      "command": "npx",
      "args": ["-y", "@executeautomation/playwright-mcp-server"]
    }
  }
}
```

## 注意事项

1. **API密钥安全**：
   - 不要将包含API密钥的配置提交到Git
   - 使用`.gitignore`排除敏感配置文件
   - 可以使用环境变量代替硬编码

2. **版本兼容**：
   - 注意工具版本兼容性
   - 定期更新配置文件格式

3. **权限管理**：
   - MCP服务器可能需要特定权限
   - 仔细审查授权的工具和操作

4. **备份配置**：
   - 重要配置及时备份
   - 可以使用Git管理（排除敏感信息）

## 推荐MCP服务器

| 服务器名称 | 功能 | 安装命令 |
|-----------|------|---------|
| context7 | 获取最新库文档 | `npx -y @context7/mcp-server` |
| playwright | 浏览器自动化 | `npx -y @executeautomation/playwright-mcp-server` |
| open-websearch | 网络搜索 | `npx -y @agentikit/open-websearch` |
| deepwiki | DeepWiki文档获取 | `npx -y @modelcontextprotocol/server-deepwiki` |

## 参考资源

- [Claude Code文档](https://docs.anthropic.com/claude-code)
- [MCP协议规范](https://modelcontextprotocol.io/)
- [MCP服务器目录](https://github.com/modelcontextprotocol/servers)

## 来源

网友那拿的（AI开发工具配置实例）

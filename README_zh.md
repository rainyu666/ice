# ICE (Intelligent Concurrency Examiner) 项目

[English](README.md) | 简体中文

本项目结合了人工智能（大语言模型）与静态代码分析技术（如 Joern），为嵌入式C项目提供自动化的缺陷检测（Defect Detection）能力。

## 🚀 前置环境要求

在本地运行本项目之前，请确保您的系统满足以下要求并安装配置好基础环境：

*   **操作系统**: **Ubuntu 24.04**（以兼容底层的核心 C 库 GLIBC 2.38 及相关编译依赖）
*   **Node.js**: v20 或更高版本（推荐使用最新的 LTS 长期支持版）
*   **Bun**: v1.3.13 或更高版本（用于极其快速的前端构建和包管理）
*   **Python**: v3.11 或 v3.12（用于运行后端的评测脚本和 MCP 核心服务端）
*   **Joern**: v4.0.450 或更高版本，并将其路径添加到系统的系统环境变量（PATH）中。

## 🛠️ 安装指南

1. **安装 Node.js 与 Bun**
   本项目需要 Node.js（推荐 v20+）以及 Bun 包管理器：
   ```bash
   # 1. 安装 Node.js (推荐通过 nvm 管理，或者直接通过 apt 安装)
   # 如果在 Ubuntu 24.04 上，您可以直接运行：
   sudo apt update
   sudo apt install -y nodejs npm

   # 2. 安装 Bun
   curl -fsSL https://bun.sh/install | bash
   source ~/.bashrc
   ```

2. **安装 Joern**
   Joern 是将 C/C++ 代码解析为代码属性图（CPG）的核心依赖：
   ```bash
   # 下载并赋予安装脚本执行权限
   curl -L "https://github.com/joernio/joern/releases/latest/download/joern-install.sh" -o joern-install.sh
   chmod +x ./joern-install.sh
   # 运行安装向导
   ./joern-install.sh
   # 安装完成后，请确保根据终端提示将 joern-cli 路径加入到外层环境变量中（如 ~/.bashrc）
   ```

3. **安装项目前端及 Node 依赖项**  
   我们使用 `bun` 来快速管理 monorepo（多包工作区）。请在项目根目录下直接运行：
   ```bash
   bun install
   ```

## 💻 启动说明

启动请运行：

```bash
bun run dev
```

运行后，在对话/提示词输入框输入：/connect，用于接入 LLM API。

完成连接后，选择对应的模型，后续即可开始测试。

*(注意：启动后请稍等几秒钟，等待后台的mcp-server进程完全初始化并挂载成功。)*

## 📚 数据集说明

为评估 ICE 在中断并发缺陷检测中的有效性，本项目使用 `Dataset_RealProjects/` 下的数据集：包含由 42 个标注样例（AV、CIRE、MWDR）构成的 IDCBench 基准集，以及 3 个公开的真实嵌入式开源项目；另外 3 个航天工程项目不可公开。

## 🧪 快速测试与评测样例

**请直接在对话/提示词输入框内，输入以下这段原话：**

> 对 Dataset_RealProjects/IDCBench/AV/svp_simple_001 进行缺陷检测

**预期的执行结果：**
智能代理（Agent）将会自动完成以下无缝工作流：
1. 自动在工作区定位到 `IDCBench/AV/svp_simple_001/` 目录下的测试向量/源码。
2. 在后台静默调用 Joern 等静态分析工具，对目标源码的特征结构进行代码属性图（CPG）提取。
3. 结合 LLM（大模型）后端引擎，分析代码中是否存在特定的安全漏洞（如缓冲区溢出、空指针解引用等）。
4. 最终在聊天窗口中输出一份结构化的分析报告，包含：具体的缺陷位置、具体的漏洞类型等。

---
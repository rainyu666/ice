# ICE (Intelligent Concurrency Examiner) Project

English | [简体中文](README_zh.md)

This project combines Artificial Intelligence (Large Language Models) with static code analysis techniques (such as Joern) to provide automated defect detection capabilities for embedded C projects.

## 🚀 Prerequisites

Before running this project locally, please ensure your system meets the following requirements and the fundamental environment is installed and configured:

*   **Operating System**: **Ubuntu 24.04** (to ensure compatibility with the underlying core C library GLIBC 2.38 and related compilation dependencies)
*   **Node.js**: v20 or higher (the latest LTS version is recommended)
*   **Bun**: v1.3.13 or higher (transpiler for blazingly fast frontend building and package management)
*   **Python**: v3.11 or v3.12 (used to run backend evaluation scripts and the MCP core server)
*   **Joern**: v4.0.450 or higher, and its path MUST be added to the system's environment variable (PATH).

## 🛠️ Installation Guide

1. **Install Node.js & Bun**
   This project requires Node.js (v20+ recommended) and the Bun package manager:
   ```bash
   # 1. Install Node.js (via apt in Ubuntu 24.04, or nvm)
   sudo apt update
   sudo apt install -y nodejs npm

   # 2. Install Bun
   curl -fsSL https://bun.sh/install | bash
   source ~/.bashrc
   ```

2. **Install Joern**
   Joern is the core dependency for parsing C/C++ code into Code Property Graphs (CPG), and it must be installed on your system:
   ```bash
   # Download and grant execution permission to the installation script
   curl -L "https://github.com/joernio/joern/releases/latest/download/joern-install.sh" -o joern-install.sh
   chmod +x ./joern-install.sh
   # Run the installation wizard
   ./joern-install.sh
   # After installation, make sure to add the joern-cli path to your environment variables according to the terminal prompts (e.g., in ~/.bashrc)
   ```

3. **Install Project Frontend & Node Dependencies**  
   We use `bun` to quickly manage the monorepo workspace. Please run the following command directly in the root directory:
   ```bash
   bun install
   ```

## 💻 Start Instructions

To start the application, simply run:

```bash
bun run dev
```

After it starts, enter /connect in the chat/prompt input box to connect to the LLM API.

Then select the target model, and you can proceed with testing.

*(Note: After starting, please wait a few seconds for the background `mcp-server` process to fully initialize and mount successfully.)*

## 📚 Dataset Overview

To evaluate ICE for interrupt-driven concurrency defect detection, this project uses the datasets under `Dataset_RealProjects/`: the IDCBench benchmark with 42 annotated cases (AV, CIRE, MWDR), plus 3 public real embedded open-source projects; the other 3 aerospace engineering projects cannot be publicly released.

## 🧪 Quick Test & Evaluation Example

**Please directly enter the following exact text into the chat/prompt input box:**

> Perform defect detection on Dataset_RealProjects/IDCBench/AV/svp_simple_001

**Expected Execution Results:**
The intelligent Agent will automatically complete the following seamless workflow:
1. Automatically locate the test vectors/source code under the `IDCBench/AV/svp_simple_001/` directory in the workspace.
2. Silently invoke static analysis tools (like Joern) in the background to extract the Code Property Graph (CPG) of the target source code.
3. Combine with the LLM backend engine to analyze whether specific security vulnerabilities (such as Buffer Overflows, Null Pointer Dereferences, etc.) exist in the code.
4. Finally, output a structured analysis report in the chat window, including: specific defect locations, specific vulnerability types, etc.

---

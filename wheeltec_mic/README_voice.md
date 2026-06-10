# Wheeltec 语音功能包说明

本目录包含完整的语音交互功能模块，支持语音控制和AI对话两种模式。

## 功能包概述

### 1. wheeltec_mic_ros2
**作用**：麦克风阵列硬件驱动包
- 负责M2麦克风阵列的硬件初始化和数据采集
- 提供语音唤醒、录音、音频流传输功能
- 支持语音增强和噪声抑制

**话题**：
- `/voice_words`：发布识别到的语音文本

### 2. wheeltec_mic_aiui
**作用**：科大讯飞AIUI语音识别服务
- 集成科大讯飞云端语音识别功能
- 提供两种工作模式：
  - **命令识别模式**（command_recognition）：预定义语音指令控制
  - **对话模式**（chat_service）：AI智能对话

**核心节点**：
- `command_recognition`：匹配预定义语音指令，控制机器狗运动
- `chat_service`：将语音文本发送到AI大模型处理

**话题**：
- 订阅：`/voice_words`（语音识别结果）
- 发布：`/feedback_words`（AI回复文本）

### 3. tts_make_ros2
**作用**：科大讯飞TTS语音合成包
- 将文本转换为语音并播放
- 支持离线合成（使用xiaoyan音色）
- 自动清理播放后的音频文件
- 预处理文本，去除Markdown格式符号

**话题**：
- 订阅：`/feedback_words`（需要合成的文本）

### 4. ollama_ros_chat
**作用**：Ollama AI大模型ROS2接口
- 连接本地或云端Ollama服务
- 提供自然语言对话能力
- 支持多种开源大模型（如deepseek-r1、qwen等）

**服务**：
- `/chat`：接收文本输入，返回AI回复

**配置文件**：
- `config/ollama_params.yaml`：模型选择、温度参数、历史长度等

### 5. wheeltec_mic_msg
**作用**：自定义消息类型定义
- 定义语音功能相关的ROS2消息和服务类型

## 启动使用

### 方式一：完整语音交互系统（推荐）
包含语音识别 + AI对话 + 语音合成的完整流程：

```bash
cd ~/zsibot_L1_ws
source install/setup.bash

# 确保Ollama服务已启动
# ollama serve  # 在另一个终端运行, snap 安装的 ollama 服务已经自动启动了

# 启动完整语音交互
ros2 launch ollama_ros_chat ollama_chat.launch.py   
ros2 launch wheeltec_mic_aiui aiui_chat_tts.launch.py
```

**使用步骤**：
1. 说出唤醒词"你好小京"
2. 听到"叮"提示音后说话
3. 系统自动识别 → AI处理 → 语音回复

### 方式二：仅语音控制模式
使用预定义指令控制机器狗运动：

```bash
ros2 launch wheeltec_mic_ros2 base.launch.py
ros2 launch wheeltec_mic_aiui mic_start.launch.py
```

**支持指令**（见 VOICE_COMMANDS.md）：
- 前进、后退、左转、右转
- 趴下、站立
- 切换步态、加速、减速等

### 方式三：仅AI对话模式
不启动TTS，仅在终端查看对话：

```bash
# 终端1：启动麦克风和识别
ros2 launch wheeltec_mic_aiui aiui_chat.launch.py

# 终端2：启动ai服务
ros2 launch ollama_ros_chat ollama_chat.launch.py   
```

## 配置说明

### Ollama模型配置
修改 `ollama_ros_chat/ollama_ros_chat/config/ollama_params.yaml`：

```yaml
base_url: "http://localhost:11434/v1"  # Ollama服务地址
use_model: "deepseek-r1:1.5b"          # 使用的模型
temperature: 0.5                        # 生成温度（0-1）
history_length: 10                      # 对话历史长度
```

**推荐模型**：
- `deepseek-r1:1.5b`：轻量级，速度快（1.1GB）
- `qwen2.5:3b`：中文优化，效果更好（1.9GB）
- `qwen2.5:7b`：最佳效果，需要较好性能（4.7GB）

安装模型：
```bash
ollama pull deepseek-r1:1.5b
# 或
ollama pull qwen2.5:3b
```

### TTS参数配置
TTS节点使用科大讯飞离线合成引擎，主要参数在代码中设置：
- 音色：xiaoyan（青年女声）
- 语速：50（可调范围0-100）
- 音量：50（可调范围0-100）

需要修改时编辑 `tts_make_ros2/src/tts_make.cpp`。

## 系统架构

```
麦克风阵列(wheeltec_mic_ros2)
    ↓ /voice_words
语音识别(wheeltec_mic_aiui)
    ↓ /voice_words
    ├─→ 命令识别(command_recognition) → 控制指令
    └─→ 对话服务(chat_service)
            ↓ /chat service
        Ollama AI(ollama_ros_chat)
            ↓ /feedback_words
        TTS合成(tts_make_ros2) → 语音播放
```

## 故障排查

### 1. 语音识别无响应
- 检查麦克风连接：`lsusb` 查看设备
- 检查AIUI配置：确认AppID、APIKey有效
- 查看日志：`ros2 topic echo /voice_words`

### 2. AI无回复
- 确认Ollama服务运行：`curl http://localhost:11434`
- 检查模型已下载：`ollama list`
- 查看服务状态：`ros2 service list | grep chat`

### 3. TTS无声音
- 检查音频输出：`aplay -l` 查看声卡
- 测试播放：`aplay /usr/share/sounds/alsa/Front_Center.wav`
- 查看MSP登录状态（查看日志中的"MSP登录成功"）

### 4. 音频文件未清理
- TTS节点会自动删除播放后的文件
- 手动清理：`rm -f ~/zsibot_L1_ws/audio/*.wav`

## 依赖安装

```bash
# Ollama（用于AI对话）
sudo snap install ollama

# 音频播放工具
sudo apt install alsa-utils

# 科大讯飞SDK
# 已包含在 msc/ 目录中，无需额外安装
```

## 参考文档

- `AIUI平台语音功能使用手册_不依赖小车_ROS2.pdf`：AIUI详细配置
- `M2系列麦克风阵列ROS2功能使用教程v2.5_20250723.pdf`：麦克风硬件使用
- `ROS2 TTS功能使用说明.pdf`：TTS功能详解
- `VOICE_COMMANDS.md`：完整语音指令列表

## 开发说明

### 文本预处理
TTS节点会自动清理以下符号，使语音更自然：
- Markdown加粗：`**文本**` → `文本`
- 括号：`(内容)`、`（内容）` → 移除
- 方括号：`[内容]` → 移除
- 竖线：`|` → 移除
- 井号：`#` → 移除

### 自定义语音指令
编辑 `wheeltec_mic_aiui/node/command_recognition.cpp`，在 `CommandsHashMap()` 函数中添加：

```cpp
commands["你的指令"] = YOUR_COMMAND_CODE;
```

然后重新编译：
```bash
colcon build --packages-select wheeltec_mic_aiui
```

## 许可证

遵循各子包的开源协议，科大讯飞SDK遵循其商业使用条款。

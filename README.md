# wheeltec_mic — 机器人语音交互系统

基于 Wheeltec 麦克风阵列的 ROS2 语音交互项目，支持在线/离线语音识别、AI 对话、语音合成、机器人动作控制。

## 功能包总览

| 功能包 | 类型 | 用途 |
|--------|------|------|
| `wheeltec_mic_msg` | 消息/服务定义 | 自定义 msg/srv，被其他包依赖 |
| `serial` | C++ 库 | 串口通信库，供 mic 节点调用 |
| `wheeltec_mic_ros2` | C++ 节点 | **离线**语音识别 + 命令控制 |
| `wheeltec_mic_aiui` | C++ 节点 | **在线**语音识别(AIUI) + 命令控制/AI对话 |
| `ollama_ros_chat` | Python 节点 | 本地大模型(Ollama)对话服务 |
| `ollama_ros_msgs` | 服务定义 | Chat.srv，ollama_ros_chat 依赖 |
| `tts_make_ros2`(包名`tts`) | C++ 节点 | 讯飞离线 TTS 语音合成 |
| `action_library` | Python 库 | 机器人肢体动作控制(手臂/手势/舞蹈) |

---

## 功能一：在线语音 AI 对话

> 唤醒后语音提问，AI 生成回答并自动语音播报。

**涉及功能包**：`ollama_ros_chat` + `wheeltec_mic_aiui`

**启动命令**（两个终端）：
```bash
# 终端1：启动 Ollama 对话服务（需先确保 Ollama 已运行且模型已拉取，ollama本地配合参考csdn收藏文章配置）
ros2 launch ollama_ros_chat ollama_ros_chat.launch.py

# 终端2：启动 AIUI 语音识别 + 聊天客户端
ros2 launch wheeltec_mic_aiui aiui_chat_tts.launch.py
```

**实现原理**：麦克风唤醒 → AIUI 在线语音识别为文字 → `chat_service` 节点调用 `chat_service` ROS2 服务 → Ollama 大模型生成回复 → AIUI 云端 TTS 合成语音 → PCMPlayer 播放

**数据流**：wheeltec_mic.cpp唤醒（awake_flag） → Processor.cpp 录音并识别（voice_words） → chat_service.cpp 处理识别结果(可以用来截断做动作控制也可以直接发给ollama大模型做回答)（chat_service） → ollama_service.py 配合prompt做出回答 → chat_service.cpp 接受大模型反馈（feedback_words） → Processor.cpp 云端tts合成语音 → PCMPlayer 播放

**可调参数**：

| 参数 | 文件 | 默认值 | 说明 |
|------|------|--------|------|
| `base_url` | `ollama_params.yaml` | `http://localhost:11434/v1` | Ollama API 地址，换云服务时改为对应 URL |
| `api_key` | `ollama_params.yaml` | `ollama` | 本地随意，云服务填真实 key |
| `use_model` | `ollama_params.yaml` | `qwen2.5:3b` | 模型名，需提前 `ollama pull`；留空自动选第一个 |
| `temperature` | `ollama_params.yaml` | `0.5` | 生成随机性，越低越确定 |
| `history_length` | `ollama_params.yaml` | `10` | 对话历史保留轮数，增大占更多 token |
| `knowledge_base_path` | `ollama_params.yaml` | 见下方⚠️ | 知识库 .txt 文件目录路径 |
| `voice_name` | `aiui.cfg` → tts 段 | `x4_lingxiaoying_em_v2` | TTS 发音人，在讯飞控制台添加后替换 |
| `vad_bos` | `aiui.cfg` → vad 段 | `10000`(ms) | VAD 前端超时，说话停顿多久结束识别 |

**更换发音人**：登录 https://www.xfyun.cn → 应用 → 在线语音合成 → 添加授权发音人 → 复制 vcn 名 → 替换 `aiui.cfg` 中 `voice_name`

**注意事项**：
- Ollama 服务需提前启动：`ollama serve`，模型需提前拉取：`ollama pull qwen2.5:3b`
- `aiui_chat_tts.launch.py` 中 TTS 节点已注释，AIUI 自身完成云端合成和播报，无需额外启 TTS 节点
- 知识库路径硬编码了旧路径，需修改为当前实际路径（见底部⚠️）

---

## 功能二：在线语音命令控制

> 唤醒后说控制指令（前进/后退/左转/右转/停止/休眠/跟我走/去I点 等），机器人执行对应动作。

**涉及功能包**：`wheeltec_mic_aiui`

**启动命令**：
```bash
ros2 launch wheeltec_mic_aiui mic_start.launch.py
```

**实现原理**：麦克风唤醒 → AIUI 识别文字 → `command_recognition` 匹配命令关键词 → 发布 `/cmd_vel`、`/robot/posture_command` 等话题控制机器人（当前机器人的控制接口没有放进来，所以只是做了语音识别和关键词匹配和播报，后期机器人动作多的话，可以把chat_service那里的语音截断控制放到这里来实现）

**数据流**：wheeltec_mic.cpp 唤醒（awake_flag） → Processor.cpp 录音并在线识别（voice_words） → command_recognition.cpp 匹配命令关键词 → feedback_words_pub 播报反馈（feedback_words） → Processor.cpp 云端TTS合成 → PCMPlayer 播放

**支持的语音指令**（需以唤醒词开头，如"小格"）：

| 类别 | 指令示例 | 效果 |
|------|----------|------|
| 运动 | 前进/后退/左转/右转/停 | 发布 /cmd_vel 速度命令 |
| 姿态 | 站起来/趴下/爬行 | 发布 /robot/posture_command |
| 跟随 | 跟我走/停止跟随 | 启动/停止激光跟随 |
| 导航 | 去I点/去J点/去K点 | 发送预置导航点目标 |
| 休眠 | 休眠 | 进入休眠状态 |

**可调参数**：串口参数在 launch 文件中修改（`usart_port_name`、`serial_baud_rate`）

---

## 功能三：离线语音命令控制

> 不依赖网络的离线语音识别 + 命令控制，适合无网环境。

**涉及功能包**：`wheeltec_mic_ros2`

**启动命令**（两个终端）：
```bash
# 终端1：启动麦克风驱动 + 离线识别引擎
ros2 launch wheeltec_mic_ros2 mic_init.launch.py

# 终端2：启动命令识别 + 反馈
ros2 launch wheeltec_mic_ros2 base.launch.py
```

**实现原理**：麦克风唤醒 → `call_recognition` 调用离线 ASR 服务 → `command_recognition` 匹配命令 → 控制机器人动作 + 音频反馈

**数据流**：wheeltec_mic.cpp 唤醒（awake_flag） → call_recognition.cpp 触发录音并调用离线识别服务（get_offline_result_srv） → voice_control.cpp 录音 + 讯飞离线ASR识别返回文字（voice_words） → command_recognition.cpp 匹配命令关键词 → 发布 /cmd_vel 控制运动 / 发布 /robot/posture_command 控制姿态 / 发布 feedback_words 语音反馈 → node_feedback.cpp 播放对应状态音频（aplay）

**可调参数**：

| 参数 | 文件 | 默认值 | 说明 |
|------|------|--------|------|
| `confidence` | `recognition_params.yaml` | `18` | 离线识别置信度阈值(0-100)，越低越容易误识别 |
| `seconds_per_order` | `recognition_params.yaml` | `15` | 每次录音最大时长(秒) |
| `confidence_threshold` | `base.launch.py` | `20` | call_recognition 的置信度，launch 层覆盖 |
| `time_per_order` | `base.launch.py` | `10` | call_recognition 的最大识别时长(0-10s) |
| `line_vel_x` | `recognition_params.yaml` | `0.2` | 语音控制前进速度(m/s) |
| `ang_vel_z` | `recognition_params.yaml` | `0.2` | 语音控制转向角速度(rad/s) |
| `radar_range` | `recognition_params.yaml` | `0.75` | 雷达避障阈值(米) |
| `if_akm_yes_or_no` | `recognition_params.yaml` | `"no"` | 阿克曼车型填 yes，其他填 no |
| `I/J/K_position_*` | `recognition_params.yaml` | 见文件 | 语音导航预置点坐标 |
| `appid` | `mic_init.launch.py` | `26fbcaa3` | 讯飞离线 ASR AppID |

---

## 功能四：离线 TTS 语音合成

> 订阅 `feedback_words` 话题，将文字合成为语音并播放。可独立使用或与其他功能组合。

**涉及功能包**：`tts_make_ros2`（包名 `tts`）

**启动命令**：
```bash
ros2 launch tts tts_make.launch.py
```

**实现原理**：订阅 `feedback_words` 话题 → 调用讯飞离线 TTS SDK 合成 WAV → `aplay` 播放

**数据流**：外部节点发布文字（feedback_words） → tts_make.cpp 订阅并接收文字 → 调用讯飞离线TTS SDK（QTTSSessionBegin/TextPut/AudioGet）合成WAV音频 → 写入临时文件 → aplay 播放 → 删除临时文件

**独立测试**：
```bash
# 启动后发布文字测试
ros2 topic pub --once /feedback_words std_msgs/msg/String "{data: '你好，我是小京'}"
```

**可调参数**（`tts_params.yaml`）：

| 参数 | 默认值 | 说明 |
|------|--------|------|
| `voice_name` | `xiaoyan` | 发音人名称，需在讯飞控制台申请授权 |
| `volume` | `50` | 音量 [0-100] |
| `speed` | `50` | 语速 [0-100] |
| `pitch` | `50` | 音调 [0-100] |
| `rdn` | `0` | 数字发音方式：0数值优先 1完全数值 2完全字符串 3字符串优先 |
| `sample_rate` | `16000` | 采样率，支持 16000/8000 |
| `appid` | `26fbcaa3` | 讯飞 TTS AppID |

**配置前提**：需按 `ROS2 TTS功能使用说明.pdf` 完成讯飞 SDK 配置和发音人授权

---

## 功能五：机器人肢体动作控制

> 通过 WebSocket 控制机器人手臂、手势、舞蹈等动作，被 `chat_service` 在线对话中"握手"等关键词触发。

**涉及文件**：`action_library/` 目录

| 文件 | 功能 | 连接目标 |
|------|------|----------|
| `control_robot_arms.py` | 双臂关节角度控制 | `ws://10.192.1.2:5000` |
| `control_robot_hands.py` | 手指/手势控制(张开/握拳/握手) | `ws://10.192.1.2:5000` |
| `custom_action_lib.py` | 组合动作(握手+复位) | 组合调用 arms+hands |
| `robot_motions_lib.py` | 23 种预设动作(鞠躬/挥手/点头/鼓掌等) | `ws://10.192.1.2:5000` |
| `robot_dances_lib.py` | 14 种预设舞蹈 | `ws://10.192.1.2:5000` |

**数据流**：chat_service.cpp 匹配到"握手"等动作关键词 → custom_action_lib.py 调用组合动作 → control_robot_arms.py 通过WebSocket发送关节角度（ws://10.192.1.2:5000） → control_robot_hands.py 通过WebSocket发送手指指令 → 机器人执行动作

**硬编码值需注意**：
- `WS_URL = "ws://10.192.1.2:5000"` — 机器人控制服务地址，需根据实际 IP 修改
- `ROBOT_SN = "HU_D04_01_325"` — 机器人序列号，需匹配实际设备

**调用示例**（在 Python 中）：
```python
from action_library.custom_action_lib import make_gesture
make_gesture("shake")  # 触发握手动作后复位
```

---

## 部署到机器人

**脚本**：`scripts/deploy_voice_to_robot.sh`

一键部署离线语音控制到 M1 机械狗：打包 → SCP 传输 → 远程编译 → 配置串口 udev 规则 → 创建 systemd 开机自启服务

```bash
# 在 workspace 根目录执行
bash src/wheeltec_mic/scripts/deploy_voice_to_robot.sh
```

**脚本内硬编码值**：
- `ROBOT_IP="192.168.234.1"` — 机器人 IP
- `ROBOT_USER="robot"` / `ROBOT_PASSWORD="bot"` — SSH 凭据
- 仅编译 `wheeltec_mic_msg serial wheeltec_mic_ros2`（离线控制），不含 AIUI/Ollama 包

**部署后管理**：
```bash
sudo systemctl start/stop/restart voice_control.service   # 启停服务
sudo systemctl status voice_control.service                # 查看状态
sudo journalctl -u voice_control.service -f                # 查看日志
```

---

## 话题/服务速查

| 名称 | 类型 | 发布方 → 订阅方 | 说明 |
|------|------|------------------|------|
| `awake_flag` | Int8 | wheeltec_mic → 多节点 | 唤醒标志 |
| `voice_flag` | Int8 | wheeltec_mic → command | 麦克风就绪 |
| `awake_angle` | UInt32 | wheeltec_mic → command | 声源方向角 |
| `voice_words` | String | AIUI/voice_control → command/chat | 识别文字 |
| `feedback_words` | String | command/chat → tts/AIUI | TTS 播报文字 |
| `/cmd_vel` | Twist | command → 机器人底盘 | 速度控制 |
| `/robot/posture_command` | Int32 | command → 机器人 | 姿态(1站2趴3爬) |
| `chat_service` | Chat.srv | chat_service → ollama_server | AI 对话请求 |

---

## ⚠️ 需修改的硬编码路径

项目从其他环境迁移过来，多处硬编码了旧路径，**必须修改后才能运行**：

| 旧路径 | 出现位置 | 改为 |
|--------|----------|------|
| `/home/guest/offline_chat/...` | `Processor.h:49` `CURRENT_PATH` | 当前 wheeltec_mic_aiui 实际路径 |
| `/home/guest/offline_chat/...` | `chat_service.cpp` 多处音频路径 | 当前音频文件实际路径 |
| `/home/guest/offline_chat/...` | `user_interface.h:21` `source_path` | 当前 wheeltec_mic_ros2 实际路径 |
| `/home/guest/.../database` | `ollama_params.yaml` `knowledge_base_path` | 当前知识库实际路径 |
| `/home/guest/.../database` | `ollama_service.py:43` 默认参数 | 同上 |
| `/home/zl/zsibot_L1_ws` | `call_recognition.cpp` 注释中 | 可忽略(已注释) |
| `/dev/ttyCH343USB0` | 代码默认串口 | launch 中已覆盖为 `/dev/wheeltec_mic_test` |

> **建议**：全局搜索 `/home/guest` 和 `CURRENT_PATH`，统一替换为当前环境路径。

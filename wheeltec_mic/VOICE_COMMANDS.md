# M1机械狗语音命令配置说明

## 📋 可识别的命令词（人体追踪模式）

### 1. 基础运动
- **小狗前进** - 向前移动2秒
- **小狗后退** - 向后移动2秒
- **小狗停** - 停止运动

### 2. 姿态控制
- **小狗站起来** - 站立姿态（发布 posture_command = 1）
- **小狗趴下** - 趴下姿态（发布 posture_command = 2）
- **小狗匍匐** - 匍匐姿态（发布 posture_command = 3）

### 3. 旋转控制
- **小狗原地转圈** - 原地360度旋转
- **小狗向后转** - 向后转180度
- **小狗向左转** - 向左转90度
- **小狗向右转** - 向右转90度
- **小狗左转** - 向左转1秒
- **小狗右转** - 向右转1秒

### 4. 人体追踪
- **小狗跟着我** / **小狗开始跟随** - 启动人体追踪（设置 target_track_id = 0）
- **小狗停止跟随** - 停止人体追踪（设置 target_track_id = -1）

## 🔌 发布的话题

语音命令会发布到以下话题：

- `/robot/posture_command` (Int32) - 姿态控制命令
  - 1 = 站立
  - 2 = 趴下
  - 3 = 匍匐
  
- `/cmd_vel` (Twist) - 速度控制
  - linear.x: 前进/后退速度 (±0.3 m/s)
  - angular.z: 旋转速度 (±0.5 rad/s)
  
- `/target_track_id` (Int32) - 人体追踪目标ID
  - 0 = 追踪第一个检测到的人
  - -1 = 停止追踪

## 🚀 使用方法

### 1. 启动语音控制节点
```bash
# 启动语音识别
ros2 launch wheeltec_mic_ros2 mic_init.launch.py

# 或者启动完整的语音控制系统
ros2 launch wheeltec_mic_ros2 base.launch.py
```

### 2. 启动人体追踪系统
```bash
# 启动完整的人体跟随功能
ros2 launch person_following person_following.launch.py
```

### 3. 测试语音命令
```bash
# 唤醒语音助手
说："小微小微"

# 然后说出命令
说："小狗站起来"      # 机器狗会站立
说："小狗跟着我"      # 启动人体追踪
说："小狗向后转"      # 转180度
说："小狗停止跟随"    # 停止追踪
```

## 🔧 配置文件位置

- **语法定义**: `config/call.bnf`
- **命令处理**: `src/command_recognition.cpp`
- **头文件**: `include/command_recognition.h`
- **构建目录**: `config/msc/res/asr/GrmBuilld/`

## 📝 修改步骤

1. **编辑 call.bnf** - 添加/修改命令词
2. **清理构建缓存**: `rm -rf config/msc/res/asr/GrmBuilld/*`
3. **重新编译**: `colcon build --packages-select wheeltec_mic_ros2`
4. **重启节点**: 重新启动 mic_init.launch.py

## ⚠️ 注意事项

1. **命令格式固定**: 必须以"小狗/机械狗/狗子"开头
2. **中文识别**: 仅支持中文命令
3. **置信度阈值**: 默认20，可在调用服务时设置
4. **识别时长**: 默认5秒，唤醒后开始录音
5. **运动持续时间**: 前进/后退默认持续2秒，转向持续1秒
6. **人体追踪**: 需要配合 person_following 包使用

## 🎯 命令实现细节

### 姿态控制实现
```cpp
void Command::publish_posture(int posture_cmd){
    std_msgs::msg::Int32 msg;
    msg.data = posture_cmd;
    posture_pub->publish(msg);
}
```

### 速度控制实现
```cpp
void Command::publish_velocity(double linear_x, double angular_z, double duration){
    // 持续发送指定时间的速度命令
    // duration=0 时单次发送，>0 时持续发送并自动停止
}
```

### 旋转控制实现
```cpp
void Command::rotate_angle(double angle_deg, double angular_speed){
    // 根据角度和角速度计算持续时间
    // 自动判断旋转方向
}
```

## 📊 完整命令列表

| 命令 | 功能 | 话题 | 数据 |
|------|------|------|------|
| 小狗站起来 | 站立姿态 | /robot/posture_command | 1 |
| 小狗趴下 | 趴下姿态 | /robot/posture_command | 2 |
| 小狗匍匐 | 匍匐姿态 | /robot/posture_command | 3 |
| 小狗前进 | 前进2秒 | /cmd_vel | linear.x=0.3 |
| 小狗后退 | 后退2秒 | /cmd_vel | linear.x=-0.3 |
| 小狗左转 | 左转1秒 | /cmd_vel | angular.z=0.5 |
| 小狗右转 | 右转1秒 | /cmd_vel | angular.z=-0.5 |
| 小狗向左转 | 左转90° | /cmd_vel | 90° @ 0.5rad/s |
| 小狗向右转 | 右转90° | /cmd_vel | -90° @ 0.5rad/s |
| 小狗向后转 | 后转180° | /cmd_vel | 180° @ 0.5rad/s |
| 小狗原地转圈 | 转360° | /cmd_vel | 360° @ 0.5rad/s |
| 小狗停 | 停止运动 | /cmd_vel | 全部为0 |
| 小狗跟着我 | 启动追踪 | /target_track_id | 0 |
| 小狗停止跟随 | 停止追踪 | /target_track_id | -1 |

## 🔗 与 person_following 包集成

语音命令已完全集成到 person_following 包的控制架构中：

1. **姿态控制**: 通过 `/robot/posture_command` 话题，由 `zsl1_controller_node` 接收并执行
2. **速度控制**: 通过 `/cmd_vel` 话题，由 `zsl1_controller_node` 接收并转换为机器人运动
3. **追踪控制**: 通过 `/target_track_id` 话题，由 `yolov8_bytetrack_node` 接收并选择追踪目标

所有控制接口与 keyboard_control_node.py 和 web_control_server.py 完全一致。


#!/bin/bash
# 快速部署语音控制到M1机械狗脚本

set -e

# 颜色定义
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# 配置
ROBOT_IP="192.168.234.1"
ROBOT_USER="robot"
ROBOT_PASSWORD="bot"
WORKSPACE_NAME="voice_control_ws"

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}   M1机械狗语音控制快速部署脚本${NC}"
echo -e "${BLUE}========================================${NC}"
echo ""

# 检查是否在正确的目录
if [ ! -d "src/wheeltec_mic" ]; then
    echo -e "${RED}错误: 请在workspace根目录(/home/zl/zsibot_L1_ws)执行此脚本${NC}"
    exit 1
fi

echo -e "${YELLOW}目标机器人: $ROBOT_USER@$ROBOT_IP${NC}"
echo ""

# 提示用户确认
read -p "$(echo -e ${YELLOW}是否继续部署? [y/N]: ${NC})" -n 1 -r
echo
if [[ ! $REPLY =~ ^[Yy]$ ]]; then
    echo -e "${RED}已取消部署${NC}"
    exit 1
fi

echo ""
echo -e "${GREEN}[1/7] 测试机械狗连接...${NC}"
if ping -c 2 $ROBOT_IP > /dev/null 2>&1; then
    echo -e "${GREEN}✓ 机械狗连接正常${NC}"
else
    echo -e "${RED}✗ 无法连接到机械狗，请检查网络${NC}"
    exit 1
fi

echo ""
echo -e "${GREEN}[2/7] 准备传输文件...${NC}"
# 创建临时压缩包
echo "正在压缩wheeltec_mic包..."
cd src
tar czf /tmp/voice_control_packages.tar.gz \
    wheeltec_mic/ \
    serial/ \
    wheeltec_mic_msg/ \
    2>/dev/null || true
cd ..
echo -e "${GREEN}✓ 文件准备完成${NC}"

echo ""
echo -e "${GREEN}[3/7] 传输文件到机械狗...${NC}"
echo "这可能需要几分钟，请耐心等待..."
scp /tmp/voice_control_packages.tar.gz $ROBOT_USER@$ROBOT_IP:~/ || {
    echo -e "${RED}✗ 文件传输失败${NC}"
    exit 1
}
echo -e "${GREEN}✓ 文件传输完成${NC}"

echo ""
echo -e "${GREEN}[4/7] 在机械狗上解压并编译...${NC}"
ssh $ROBOT_USER@$ROBOT_IP 'bash -s' << 'ENDSSH'
    set -e
    echo "创建workspace..."
    mkdir -p ~/voice_control_ws/src
    
    echo "解压文件..."
    cd ~/voice_control_ws/src
    tar xzf ~/voice_control_packages.tar.gz
    rm ~/voice_control_packages.tar.gz
    
    echo "安装依赖..."
    sudo apt update > /dev/null 2>&1
    sudo apt install -y ros-humble-serial-driver libasound2-dev libcjson-dev > /dev/null 2>&1
    
    echo "编译包..."
    cd ~/voice_control_ws
    source /opt/ros/humble/setup.bash
    colcon build --packages-select wheeltec_mic_msg serial wheeltec_mic_ros2
    
    if [ $? -eq 0 ]; then
        echo "✓ 编译成功"
    else
        echo "✗ 编译失败"
        exit 1
    fi
ENDSSH

echo -e "${GREEN}✓ 编译完成${NC}"

echo ""
echo -e "${GREEN}[5/7] 配置串口权限...${NC}"
ssh $ROBOT_USER@$ROBOT_IP 'bash -s' << 'ENDSSH'
    # 添加用户到dialout组
    sudo usermod -aG dialout $USER
    
    # 配置udev规则（如果存在）
    if [ -f ~/voice_control_ws/src/wheeltec_mic/配置Linux环境文件/配置串口/91-mic-usb-serial.rules ]; then
        sudo cp ~/voice_control_ws/src/wheeltec_mic/配置Linux环境文件/配置串口/91-mic-usb-serial.rules \
            /etc/udev/rules.d/ 2>/dev/null || true
        sudo udevadm control --reload-rules
        sudo udevadm trigger
        echo "✓ udev规则已配置"
    else
        echo "⚠ udev规则文件未找到，需要手动配置"
    fi
ENDSSH

echo -e "${GREEN}✓ 权限配置完成${NC}"

echo ""
echo -e "${GREEN}[6/7] 创建systemd服务...${NC}"
ssh $ROBOT_USER@$ROBOT_IP 'bash -s' << 'ENDSSH'
    # 创建systemd服务文件
    sudo tee /etc/systemd/system/voice_control.service > /dev/null << 'EOF'
[Unit]
Description=M1 Voice Control System
After=network.target
Wants=network-online.target

[Service]
Type=simple
User=robot
Group=robot
WorkingDirectory=/home/robot/voice_control_ws

# 等待机械狗底层系统启动
ExecStartPre=/bin/sleep 10

# 启动语音控制
ExecStart=/bin/bash -c 'source /opt/ros/humble/setup.bash && source /home/robot/voice_control_ws/install/setup.bash && ros2 launch wheeltec_mic_ros2 mic_init.launch.py'

# 自动重启
Restart=on-failure
RestartSec=10

# 日志配置
StandardOutput=journal
StandardError=journal

[Install]
WantedBy=multi-user.target
EOF

    # 重新加载systemd
    sudo systemctl daemon-reload
    
    # 启用开机自启动
    sudo systemctl enable voice_control.service
    
    echo "✓ systemd服务已创建并启用"
ENDSSH

echo -e "${GREEN}✓ 服务配置完成${NC}"

echo ""
echo -e "${GREEN}[7/7] 清理临时文件...${NC}"
rm -f /tmp/voice_control_packages.tar.gz
echo -e "${GREEN}✓ 清理完成${NC}"

echo ""
echo -e "${BLUE}========================================${NC}"
echo -e "${GREEN}✅ 部署完成！${NC}"
echo -e "${BLUE}========================================${NC}"
echo ""
echo -e "${YELLOW}下一步操作：${NC}"
echo ""
echo "1. 将麦克风阵列USB插入机械狗"
echo ""
echo "2. SSH到机械狗测试："
echo -e "   ${BLUE}ssh $ROBOT_USER@$ROBOT_IP${NC}"
echo -e "   ${BLUE}sudo systemctl start voice_control.service${NC}"
echo -e "   ${BLUE}sudo systemctl status voice_control.service${NC}"
echo ""
echo "3. 查看日志："
echo -e "   ${BLUE}sudo journalctl -u voice_control.service -f${NC}"
echo ""
echo "4. 测试语音命令："
echo "   - 说'小格站起来' → 机械狗站立"
echo "   - 说'小格前进' → 机械狗前进"
echo "   - 说'小格停' → 机械狗停止"
echo ""
echo "5. 重启机械狗验证开机自启动："
echo -e "   ${BLUE}ssh $ROBOT_USER@$ROBOT_IP 'sudo reboot'${NC}"
echo ""
echo -e "${YELLOW}管理命令：${NC}"
echo -e "   启动服务: ${BLUE}sudo systemctl start voice_control.service${NC}"
echo -e "   停止服务: ${BLUE}sudo systemctl stop voice_control.service${NC}"
echo -e "   查看状态: ${BLUE}sudo systemctl status voice_control.service${NC}"
echo -e "   查看日志: ${BLUE}sudo journalctl -u voice_control.service -f${NC}"
echo ""
echo -e "${GREEN}详细文档请查看: VOICE_DEPLOYMENT_GUIDE.md${NC}"
echo ""

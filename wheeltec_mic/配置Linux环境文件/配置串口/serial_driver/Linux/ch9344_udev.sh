#!/bin/bash
# CH9344麦克风阵列，设置别名为wheeltec_mic
# 注意：CH9344有8个串口(ttyCH9344USB0-7)，麦克风通常使用第一个端口(0)

echo 'KERNEL=="ttyCH9344USB0", ATTRS{idVendor}=="1a86", ATTRS{idProduct}=="55d9", MODE:="0777", GROUP:="dialout", SYMLINK+="wheeltec_mic"' > /etc/udev/rules.d/wheeltec_mic.rules

service udev reload
sleep 2
service udev restart

echo "udev规则已创建，请检查 /dev/wheeltec_mic 是否存在"
ls -la /dev/wheeltec_mic 2>/dev/null || echo "警告：/dev/wheeltec_mic 不存在，可能需要重新插拔USB设备"

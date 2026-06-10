#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import websocket
import json
import time
import uuid


ROBOT_SN = "HU_D04_01_325"
WS_URL = "ws://10.192.1.2:5000"

def current_ms():
    """获取当前时间戳（毫秒）"""
    return int(time.time() * 1000)

def send_request(title, data=None):
    """发送请求并返回响应"""
    if data is None:
        data = {}
    
    ws = websocket.create_connection(WS_URL, timeout=5)
    try:
        request_msg = {
            "accid": ROBOT_SN,
            "title": title,
            "timestamp": current_ms(),
            "guid": uuid.uuid4().hex,
            "data": data
        }
        
        ws.send(json.dumps(request_msg))
        response = ws.recv()
        return json.loads(response)
    finally:
        ws.close()

def set_hand_pose(left_pos=None, right_pos=None, left_time=1000, right_time=1000):
    data = {}
    
    if left_pos is not None:
        data["left_mode"] = 1  # 位置时间模式
        data["left_pos"] = left_pos
        data["left_time"] = [left_time] * 6
    
    if right_pos is not None:
        data["right_mode"] = 1  # 位置时间模式
        data["right_pos"] = right_pos
        data["right_time"] = [right_time] * 6
    
    if not data:
        print("错误：至少需要设置左手或右手的位置")
        return None
    
    response = send_request("request_set_brainco2_hand_cmd", data)
    if response.get("title") == "response_set_brainco2_hand_cmd":
        result = response.get("data", {}).get("result")
        print(f"设置手势响应: {result}")
        return result == "success"
    return False

def open_hand():
    """张开双手"""
    pos = [0.0] * 6  # 所有手指张开位置
    print("正在张开双手...")
    return set_hand_pose(left_pos=pos, right_pos=pos, left_time=500, right_time=500)

def close_hand():
    """握紧双手"""
    # 各手指最大弯曲角度不同，使用近似值
    # 拇指尖: 0-1.0297, 拇指根: 0-1.5707, 其他手指: 0-1.4137
    pos = [1.0, 1.5, 1.4, 1.4, 1.4, 1.4]  # 握拳位置
    print("正在握紧双手...")
    return set_hand_pose(left_pos=pos, right_pos=pos, left_time=500, right_time=500)

def shake_hand():
    left_pos = [0.0] * 6
    right_pos = [0.3716999888420105, 1.2007999420166016, 0.2442999929189682, 0.41190001368522644, 0.5026000142097473, 0.48339998722076416]
    return set_hand_pose(left_pos=left_pos, right_pos=right_pos, left_time=500, right_time=500)

# ===== 预设手势 =====
def make_hands_gesture(gesture_name):
    """
    执行预设手势
    :param gesture_name: 手势名称，支持：open(张开), close(握拳), fist(握拳)
    """
    gesture_map = {
        "open": open_hand,
        "close": close_hand,
        "shake": shake_hand,
    }

    action = gesture_map.get(gesture_name.lower())
    if action:
        return action()
    else:
        print(f"未知手势: {gesture_name}，支持的手势: {list(gesture_map.keys())}")
        return False


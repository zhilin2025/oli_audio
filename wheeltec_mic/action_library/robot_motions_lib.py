#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import websocket
import json
import time
import uuid
import subprocess
import threading



ROBOT_SN = "HU_D04_01_325"
WS_URL = "ws://10.192.1.2:5000"
ATOMIC_MOTIONS = {
    0:  ("stand",                  "静止站立"),
    1:  ("this_way_please",        "指引请这边走"),
    2:  ("bow",                    "鞠躬"),
    3:  ("wave_greet_bye",         "挥手（你好/拜拜）"),
    4:  ("nod",                    "点头"),
    5:  ("shake_head",             "摇头"),
    6:  ("curtain_bow",            "谢幕"),
    7:  ("blow_kisses_multi",      "连续飞吻"),
    8:  ("left_hand_side_heart",   "左手侧比心"),
    9:  ("right_hand_side_heart",  "右手侧比心"),
    10: ("hand_heart",             "双手比心"),
    11: ("high_five",              "与人击掌"),
    12: ("clap",                   "鼓掌"),
    13: ("warm_up_dance",          "热场舞"),
    14: ("swag_dance",             "展臂舞"),
    15: ("idol_dance_1",           "女团舞1"),
    16: ("idol_dance_2",           "女团舞2"),
    17: ("power_up_dance",         "助力舞"),
    18: ("shake_hands",            "握手"),
    19: ("raise_and_introduce",    "上抬手介绍"),
    20: ("casual_gesture_1",       "随手比划40s"),
    21: ("casual_gesture_2",       "随手比划10s"),
    22: ("take_a_photo",           "拍照"),
}
def current_ms():
    """获取当前时间戳（毫秒）"""
    return int(time.time() * 1000)

def check_robot_state(ws):
    try:
        request_msg = {
            "accid": ROBOT_SN,
            "title": "request_robot_status",
            "timestamp": current_ms(),
            "guid": uuid.uuid4().hex,
            "data": {}
        }
        ws.send(json.dumps(request_msg))
        
        # 循环接收消息，直到收到目标响应
        while True:
            response = ws.recv()
            response_json = json.loads(response)
            
            # 忽略非目标消息
            if response_json.get("title") != "notify_robot_info":
                continue
            
            # 找到目标响应，退出循环
            break
        
        if response_json.get("title") == "notify_robot_info":
            data = response_json.get("data", {})
            result = data.get("result", [])
            
            robot_status = "unknown"
            current_mode = "unknown"
            
            # 解析机器人状态信息
            for item in result:
                name = item.get("name")
                if name == "system_info":
                    values = item.get("values", [])
                    values_dict = {v["key"]: v["value"] for v in values}
                    robot_status = values_dict.get("robot_status", "unknown")
                    current_mode = values_dict.get("mode", "unknown")
            
            print(f"机器人当前状态: {robot_status}")
            print(f"机器人当前模式: {current_mode}")
            
            if robot_status in ["IkStand", "Menu", "Walk"] or current_mode in ["IkStand", "Menu", "Walk", "move"]:
                return True
            else:
                print(f"✗ 机器人当前状态不适合进行运动，请先进入站姿模式")
                print(f"  当前状态: {robot_status}, 当前模式: {current_mode}")
                return False
        else:
            print("收到非预期的响应:", response_json.get("title"))
            return False
    
    except Exception as e:
        print(f"检查机器人状态失败: {e}")
        return False

def control_motion_mode(ws, mode):
    # 发送请求
    request_msg = {
        "accid": ROBOT_SN,
        "title": "request_set_motion_engine",
        "timestamp": current_ms(),
        "guid": uuid.uuid4().hex,
        "data": {"mode": mode}
    }
    ws.send(json.dumps(request_msg))
    
    # 循环接收消息，直到收到目标响应
    while True:
        response = ws.recv()
        response_json = json.loads(response)
        
        title = response_json.get("title")

        if title == "response_set_motion_engine":
            result = response_json.get("data", {}).get("result")
            if result == "success":
                return True
            elif result == "fail_motor":
                print(f"{mode}模式失败")
            break


def run_motion(ws, motion_name):
    # 发送请求
    request_msg = {
        "accid": ROBOT_SN,
        "title": "request_action_sync",
        "timestamp": current_ms(),
        "guid": uuid.uuid4().hex,
        "data": {"name": motion_name}
    }
    ws.send(json.dumps(request_msg))
    
    # 循环接收消息，直到收到目标响应
    while True:
        response = ws.recv()
        response_json = json.loads(response)
        
        title = response_json.get("title")

        if title == "notify_execute_atomic_motion":
            result = response_json.get("data", {}).get("result")
            if result == "success":
                return True
            else:
                print("动作执行失败")
            break
    return False

def play_audio(audio_file):
    subprocess.run([
    "ffplay",
    "-nodisp",
    "-autoexit",
    "-volume",
    "50",
    audio_file
    ])

def execute_motion(motion_name, audio_file=None):

    ws = None
    try:
        # 建立WebSocket连接
        ws = websocket.create_connection(WS_URL, timeout=5)

        if not check_robot_state(ws):
            print("警告：无法获取机器人状态，请确保机器人已开机并处于站姿模式")

        control_motion_mode(ws,1)
        time.sleep(1)
        if motion_name:
            if audio_file is not None:
                audio_thread = threading.Thread(target=play_audio, args=(audio_file,))
                audio_thread.start()
            print(f"正在执行: {motion_name}")
            if run_motion(ws, motion_name):
                print("动作执行成功")
                
        # control_motion_mode(ws,0)
        
    except Exception as e:
        print(f"控制过程中发生错误: {e}")
        return False
    finally:
        if ws:
            ws.close()

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
DANCE_MOTIONS = {
    0:  ("one_and_only_dance",       "热烈"),
    1:  ("pulp_fiction_dance",       "低俗小说"),
    2:  ("smooth_sailing_and_prosperity", "顺风顺水顺财神"),
    3:  ("all_things_grow_dance",    "万物生"),
    4:  ("popping",                  "机械舞"),
    5:  ("luv_each_other",           "相亲相爱"),
    6:  ("apt_dance",                "APT"),
    7:  ("victory_dance",            "胜利之舞"),      
    8:  ("abracadabr_dance",         "abracadabr扭胯舞"),
    9:  ("karla_ok",                 "卡啦永远OK"),    
    10: ("lets_bounce",              "来个蹦蹦"),      
    11: ("gentleman",                "gentleman"),     
    12: ("whatever",                 "管他什么音乐"),  
    13: ("solo_shake",               "孤身摇"),      
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

def control_dances_mode(ws,mode):
    # 发送请求
    request_msg = {
        "accid": ROBOT_SN,
        "title": "request_enter_dance_mode",
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

        if title == "response_enter_dance_mode":
            result = response_json.get("data", {}).get("result")
            if result == "success":
                return True
            elif result == "fail_motor":
                print(f"{mode}模式失败")
            break
    return False

def run_dance(ws, dance_name):
    # 发送请求
    request_msg = {
        "accid": ROBOT_SN,
        "title": "request_dance",
        "timestamp": current_ms(),
        "guid": uuid.uuid4().hex,
        "data": {"name": dance_name}
    }
    ws.send(json.dumps(request_msg))
    
    # 循环接收消息，直到收到目标响应
    while True:
        response = ws.recv()
        response_json = json.loads(response)
        
        title = response_json.get("title")

        if title == "notify_dance":
            result = response_json.get("data", {}).get("result")
            if result == "success":
                return True
            else:
                print("动作执行失败")
            break
    return False

def play_music(music_file):
    subprocess.run([
    "ffplay",
    "-nodisp",
    "-autoexit",
    "-volume",
    "40",
    music_file
    ])

def execute_dance(dance_name, music_file):

    ws = None
    try:
        # 建立WebSocket连接
        ws = websocket.create_connection(WS_URL, timeout=5)

        if not check_robot_state(ws):
            print("警告：无法获取机器人状态，请确保机器人已开机并处于站姿模式")
        control_dances_mode(ws,1)
        time.sleep(1)

        if dance_name:
            if music_file is not None:
                music_thread = threading.Thread(target=play_music, args=(music_file,))
                music_thread.start()
            print(f"正在执行: {dance_name}")
            if run_dance(ws, dance_name):
                print("舞蹈执行成功")
                
        control_dances_mode(ws,0)
        
    except Exception as e:
        print(f"控制过程中发生错误: {e}")
        return False
    finally:
        if ws:
            ws.close()


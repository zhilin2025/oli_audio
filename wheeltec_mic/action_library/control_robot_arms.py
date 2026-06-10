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

def send_request(ws, title, data=None):
    """发送请求并返回响应"""
    if data is None:
        data = {}
    
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
            
            # 判断是否处于站姿或行走模式
            # 站姿模式通常为 "stand" 或类似值，行走模式通常为 "walk"
            if robot_status in ["IkStand", "walking", "Walk"] or current_mode in ["stand", "walking", "walk", "move"]:
                print("✓ 机器人已处于站姿/行走模式，可以进行双臂运动")
                return True
            else:
                print(f"✗ 机器人当前状态不适合进行双臂运动，请先进入站姿模式")
                print(f"  当前状态: {robot_status}, 当前模式: {current_mode}")
                return False
        else:
            print("收到非预期的响应:", response_json.get("title"))
            return False
    
    except Exception as e:
        print(f"检查机器人状态失败: {e}")
        return False

def enter_move_mode(ws, mode=1):
    # 发送请求
    request_msg = {
        "accid": ROBOT_SN,
        "title": "request_set_move_mode",
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

        if title == "response_set_move_mode":
            result = response_json.get("data", {}).get("result")
            if result == "success":
                print("成功进入Move模式")
                return True
            elif result == "fail_motor":
                print("进入Move模式失败: 电机错误，请确保机器人已进入站姿模式")
            break
    return False


def exit_move_mode(ws):
    """退出Move模式"""
    response = send_request(ws, "request_set_move_mode", {"mode": 0})
    if response.get("title") == "response_set_move_mode":
        result = response.get("data", {}).get("result")
        if result == "success":
            print("成功退出Move模式")
            return True
    return False

def move_arms(ws, left_joints, right_joints, speed=0.4):
    """
    控制双臂运动
    :param ws: websocket连接
    :param left_joints: 左臂7个关节角度列表（弧度）
    :param right_joints: 右臂7个关节角度列表（弧度）
    :param speed: 运动速度
    """
    move_msg = {
        "left": left_joints,
        "right": right_joints,
        "speed": speed
    }
    
    # 发送运动请求
    request_msg = {
        "accid": ROBOT_SN,
        "title": "request_moveJ",
        "timestamp": current_ms(),
        "guid": uuid.uuid4().hex,
        "data": move_msg
    }
    ws.send(json.dumps(request_msg))
    
    # 循环检测数据，直到获取到目标响应
    while True:
        response = ws.recv()
        response_json = json.loads(response)
        
        title = response_json.get("title")
        if title == "response_moveJ" or title == "notify_moveJ":
            data = response_json.get("data", {})
            print(f"双臂运动响应: {data}")
            return data
        
        # 忽略非目标消息，继续等待
        # print(f"收到非目标消息: {title}")

def control_robot_arms(left_joints, right_joints, speed=0.4):
    """
    完整的双臂运动控制流程
    1. 检测机器人状态
    2. 进入Move模式
    3. 控制双臂运动
    4. 退出Move模式
    """
    ws = None
    try:
        # 建立WebSocket连接
        ws = websocket.create_connection(WS_URL, timeout=5)
        # print("成功连接到机器人")
        
        # 1. 检查机器人状态
        # print("正在检查机器人状态...")
        if not check_robot_state(ws):
            print("警告：无法获取机器人状态，请确保机器人已开机并处于站姿模式")
        
        # 2. 进入Move模式（原地Move模式）
        # print("正在进入Move模式...")
        if not enter_move_mode(ws, mode=1):
            print("错误：无法进入Move模式，请先确保机器人处于站姿模式")
            return False
        
        time.sleep(1)
        # 3. 控制双臂运动
        # print("正在控制双臂运动...")
        result = move_arms(ws, left_joints, right_joints, speed)
        if result is None:
            print("双臂运动失败")
            return False
        
        # 等待运动完成（根据实际情况调整等待时间）
        time.sleep(2)
        
        # 4. 退出Move模式
        # print("正在退出Move模式...")
        # exit_move_mode(ws)
        
        print("双臂运动控制流程完成")
        return True
        
    except Exception as e:
        print(f"控制过程中发生错误: {e}")
        return False
    finally:
        if ws:
            ws.close()

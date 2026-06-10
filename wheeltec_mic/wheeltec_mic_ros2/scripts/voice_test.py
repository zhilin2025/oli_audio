#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
简单的语音识别测试脚本
唤醒后自动识别语音并打印结果
"""

import rclpy
from rclpy.node import Node
from std_msgs.msg import Int8
from wheeltec_mic_msg.srv import GetOfflineResult
import time


class VoiceTestNode(Node):
    def __init__(self):
        super().__init__('voice_test_node')
        
        # 订阅唤醒标志位
        self.awake_sub = self.create_subscription(
            Int8,
            'awake_flag',
            self.awake_callback,
            10
        )
        
        # 创建语音识别服务客户端
        self.voice_client = self.create_client(
            GetOfflineResult,
            'get_offline_result_srv'
        )
        
        self.is_processing = False
        self.pending_future = None
        
        # 创建定时器检查服务响应
        self.timer = self.create_timer(0.1, self.check_future)
        
        self.get_logger().info('🎤 语音测试节点已启动')
        self.get_logger().info('📢 请说唤醒词 "小微小微" 来开始...')
        
    def awake_callback(self, msg):
        """唤醒标志位回调"""
        if msg.data == 1 and not self.is_processing:
            self.is_processing = True
            self.get_logger().info('✅ 检测到唤醒！正在等待你说话...')
            self.start_recognition()
    
    def start_recognition(self):
        """启动语音识别"""
        # 等待服务可用
        if not self.voice_client.wait_for_service(timeout_sec=2.0):
            self.get_logger().error('❌ 语音识别服务不可用！')
            self.is_processing = False
            return
        
        # 创建请求
        request = GetOfflineResult.Request()
        request.offline_recognise_start = True
        request.time_per_order = 5  # 5秒识别时间
        request.confidence_threshold = 20  # 置信度阈值
        
        self.get_logger().info('🎙️  开始录音，请说话（5秒内）...')
        
        # 异步调用服务
        self.pending_future = self.voice_client.call_async(request)
        self.start_time = time.time()
    
    def check_future(self):
        """定时检查Future状态"""
        if self.pending_future is None:
            return
        
        # 检查是否超时（10秒）
        if time.time() - self.start_time > 10.0:
            self.get_logger().error('❌ 服务调用超时！')
            self.pending_future = None
            self.is_processing = False
            self.get_logger().info('📢 请再次说 "小微小微" 唤醒...')
            return
        
        # 检查Future是否完成
        if self.pending_future.done():
            try:
                response = self.pending_future.result()
                self.handle_recognition_result(response)
            except Exception as e:
                self.get_logger().error(f'❌ 识别过程出错: {str(e)}')
            finally:
                self.pending_future = None
                self.is_processing = False
                self.get_logger().info('📢 请再次说 "小微小微" 唤醒...')
    
    def handle_recognition_result(self, response):
        """处理识别结果"""
        print('\n' + '='*60)
        if response.result == 'ok':
            self.get_logger().info(f'✅ 识别成功！')
            self.get_logger().info(f'📝 你说的是: 【{response.text}】')
        else:
            self.get_logger().warn(f'❌ 识别失败: {response.fail_reason}')
            self.get_logger().info(f'💡 提示: 请说话清晰，或调低置信度阈值')
        print('='*60 + '\n')


def main(args=None):
    rclpy.init(args=args)
    
    node = VoiceTestNode()
    
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        node.get_logger().info('节点已停止')
    finally:
        node.destroy_node()
        rclpy.shutdown()


if __name__ == '__main__':
    main()

#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
简化版语音识别测试脚本 - 自动调用识别服务
"""

import rclpy
from rclpy.node import Node
from std_msgs.msg import Int8, String
from wheeltec_mic_msg.srv import GetOfflineResult
import time


class SimpleVoiceTest(Node):
    def __init__(self):
        super().__init__('simple_voice_test')
        
        # 订阅语音识别结果（由voice_control发布）
        self.voice_sub = self.create_subscription(
            String,
            'voice_words',
            self.voice_callback,
            10
        )
        
        # 订阅唤醒标志
        self.awake_sub = self.create_subscription(
            Int8,
            'awake_flag',
            self.awake_callback,
            10
        )
        
        # 创建服务客户端
        self.voice_client = self.create_client(
            GetOfflineResult,
            'get_offline_result_srv'
        )
        
        self.is_processing = False
        self.pending_future = None
        self.start_time = None
        
        # 创建定时器检查Future状态
        self.timer = self.create_timer(0.1, self.check_future)
        
        self.get_logger().info('🎤 简易语音测试节点已启动')
        self.get_logger().info('📢 请说唤醒词 "小微小微"，然后说话...')
        self.get_logger().info('💡 识别时长5秒，置信度阈值20')
        
    def awake_callback(self, msg):
        """唤醒回调 - 自动调用识别服务"""
        if msg.data == 1 and not self.is_processing:
            self.get_logger().info('✅ 检测到唤醒！正在启动识别...')
            self.call_recognition_service()
    
    def call_recognition_service(self):
        """调用识别服务"""
        if not self.voice_client.wait_for_service(timeout_sec=1.0):
            self.get_logger().error('❌ 识别服务不可用')
            return
        
        request = GetOfflineResult.Request()
        request.offline_recognise_start = True
        request.time_per_order = 5  # 5秒
        request.confidence_threshold = 20
        
        self.get_logger().info('🎙️  请说话（5秒内）...')
        self.is_processing = True
        self.pending_future = self.voice_client.call_async(request)
        self.start_time = time.time()
    
    def check_future(self):
        """检查服务响应"""
        if self.pending_future is None:
            return
        
        # 超时检查
        if time.time() - self.start_time > 10.0:
            self.get_logger().error('❌ 识别超时')
            self.reset_state()
            return
        
        # 检查是否完成
        if self.pending_future.done():
            try:
                response = self.pending_future.result()
                if response.result == 'ok':
                    self.get_logger().info(f'✅ 识别成功: 【{response.text}】')
                else:
                    self.get_logger().warn(f'❌ 识别失败: {response.fail_reason}')
            except Exception as e:
                self.get_logger().error(f'❌ 出错: {str(e)}')
            finally:
                self.reset_state()
    
    def reset_state(self):
        """重置状态"""
        self.pending_future = None
        self.is_processing = False
        self.get_logger().info('📢 等待下次唤醒...')
    
    def voice_callback(self, msg):
        """语音识别结果回调（话题方式）"""
        if msg.data:
            print('\n' + '='*60)
            self.get_logger().info(f'📝 [话题]识别结果: 【{msg.data}】')
            print('='*60 + '\n')


def main(args=None):
    rclpy.init(args=args)
    node = SimpleVoiceTest()
    
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        node.get_logger().info('✋ 测试结束')
    finally:
        node.destroy_node()
        if rclpy.ok():
            rclpy.shutdown()


if __name__ == '__main__':
    main()

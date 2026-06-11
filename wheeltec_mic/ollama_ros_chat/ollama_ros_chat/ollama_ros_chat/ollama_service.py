#!/usr/bin/env python3
import rclpy
from rclpy.node import Node
from std_msgs.msg import String
from ollama_ros_msgs.srv import Chat   # 替换成你的 pkg/msg
import json
import time
import os
from typing import List, Dict, Optional

import openai   # <-- 新增

class OllamaChatNode(Node):
    def __init__(self):
        super().__init__('ollama_server')

        # 创建服务
        self.chat_service = self.create_service(
            Chat, 'chat_service', self.handle_chat_request)

        # ------------------------------------------
        # ---- 参数声明（带默认值） ----
        self.declare_parameter('base_url',   'http://localhost:11434/v1')
        self.declare_parameter('api_key',    'ollama')
        self.declare_parameter('use_model',  'qwen2.5:3b')          # 空字符串表示自动选择
        self.declare_parameter('stream',     False)
        self.declare_parameter('temperature',0.5)
        self.declare_parameter('history_length', 10)

        # ---- 读取参数 ----
        self.base_url   = self.get_parameter('base_url').value
        self.api_key    = self.get_parameter('api_key').value
        self.stream     = self.get_parameter('stream').value
        self.temperature= self.get_parameter('temperature').value
        self.history_length = self.get_parameter('history_length').value
        self.use_model  = self.get_parameter('use_model').value
        
        # ---- 用参数创建 openai 客户端 ----
        self.client = openai.OpenAI(base_url=self.base_url, api_key=self.api_key)

        # ---- 知识库 ----
        self.declare_parameter('knowledge_base_path',
            '/home/guest/offline_chat/src/wheeltec_mic/wheeltec_mic_aiui/database')
        self.kb_path = self.get_parameter('knowledge_base_path').value
        self.knowledge_docs: Dict[str, str] = {}     # 文件名 → 文档内容
        self.load_knowledge_base()

        # ---- 模型选择：优先用参数，否则自动 ----
        self.initialize_models()   # 拉取可用列表
        self.use_model = self.get_parameter('use_model').value
        if not self.use_model:
            self.select_model()        # 自动选第一个
        else:
            self.get_logger().info(f'Using forced model: {self.use_model}')
        # ---- 初始化对话历史 ----
        self.conversation_history = [
            {"role": "system", "content": "你叫小京，是京格智航自主研发的智能对话机器人，你回答问题绝对要以小京自称，回答要简短直接，适合语音播报。"}]
        self.get_logger().info('Ollama(OpenAI-compat) Chat Server initialized')
        self.base_prompt = "你叫小京，是京格智航自主研发的智能对话机器人，你回答问题绝对要以小京自称，回答要简短直接，适合语音播报。"
        # self.base_prompt = "Please answer directly, without explanation. Answer in Chinese. If you don't know the answer, say you don't know. Always refer to yourself as 小京, and you are a helpful assistant developed by Jingge Zhihang."

    # ------------ 拉取模型列表 ------------
    def initialize_models(self):
        try:
            models = self.client.models.list()
            self.available_models = [m.id for m in models.data]
            self.get_logger().info(f"Available models: {', '.join(self.available_models)}")
        except Exception as e:
            self.get_logger().error(f"Error getting models: {e}")
            self.available_models = []

    def select_model(self):
        if self.use_model:
            return  # 已经通过参数指定
        if not self.available_models:
            self.get_logger().error("No models available")
            return
        self.use_model = self.available_models[0]
        self.get_logger().info(f"Auto-selected model: {self.use_model}")

    # ======================== 知识库相关 ========================
    def load_knowledge_base(self):
        """加载 database/ 下所有 .txt 文件"""
        if not os.path.isdir(self.kb_path):
            self.get_logger().warn(f'知识库路径不存在: {self.kb_path}')
            return

        txt_files = [f for f in os.listdir(self.kb_path) if f.endswith('.txt')]
        if not txt_files:
            self.get_logger().warn(f'知识库目录为空: {self.kb_path}')
            return

        for fname in txt_files:
            fpath = os.path.join(self.kb_path, fname)
            try:
                with open(fpath, 'r', encoding='utf-8') as f:
                    content = f.read()
                self.knowledge_docs[fname] = content
                self.get_logger().info(f'已加载知识库文档: {fname} ({len(content)}字)')
            except Exception as e:
                self.get_logger().error(f'加载文档失败 {fname}: {e}')

        self.get_logger().info(f'知识库加载完成: {len(self.knowledge_docs)} 篇文档')

    def _build_kb_prompt(self) -> str:
        """构建知识库上下文提示"""
        if not self.knowledge_docs:
            return ""

        parts = ["以下是与机器人当前所处场合相关的活动背景资料，请据此回答用户关于活动的问题。对于与活动无关的日常问题，请正常回答，不要引用以下资料。"]
        for fname, content in self.knowledge_docs.items():
            title = fname.replace('.txt', '')
            parts.append(f"【{title}】\n{content}\n")
        return '\n'.join(parts)

    # ------------ 服务回调 ------------
    def handle_chat_request(self, request, response):
        try:
            user_message = request.content

            # 构建消息：base_prompt + 知识库上下文(如有) + 用户问题
            kb_context = self._build_kb_prompt()
            if kb_context:
                augmented_message = f"{self.base_prompt}\n{kb_context}\n用户问题：{user_message}"
            else:
                augmented_message = f"{self.base_prompt}\n{user_message}"

            # 对话历史只存原始问题，不存知识库上下文（避免重复膨胀）
            self.conversation_history.append({"role": "user", "content": augmented_message})
            self.get_logger().info(f"Received: {user_message}")

            ts = time.time()
            reply = self.get_response(self.conversation_history)
            self.get_logger().info(f"Response ({time.time()-ts:.2f}s): {reply}")

            if reply:
                # 存储回复时，将用户消息替换回原始问题（去重知识库上下文）
                self.conversation_history[-1] = {"role": "user", "content": user_message}
                self.conversation_history.append({"role": "assistant", "content": reply})
                self.conversation_history = self.process_data(self.conversation_history)

                response.content = reply
                response.model   = self.use_model
                response.is_done = True
            else:
                # 请求失败则回滚历史
                self.conversation_history.pop()
                response.content = "Error processing request"
                response.model   = self.use_model
                response.is_done = False
        except Exception as e:
            self.get_logger().error(f"Error processing request: {e}")
            response.content = "Error processing request"
            response.model   = self.use_model or ""
            response.is_done = False
        return response

    # ------------ 真正调用大模型 ------------
    def get_response(self, messages: List[Dict[str, str]]) -> Optional[str]:
        try:
            if not self.stream:
                # 非流式
                resp = self.client.chat.completions.create(
                    model=self.use_model,
                    messages=messages,
                    temperature=self.temperature,
                    stream=False
                )
                return resp.choices[0].message.content

            # 流式（如需）
            full = ""
            for chunk in self.client.chat.completions.create(
                    model=self.use_model,
                    messages=messages,
                    temperature=self.temperature,
                    stream=True
                    ):
                delta = chunk.choices[0].delta.content or ""
                full += delta
            return full

        except Exception as e:
            self.get_logger().error(f"OpenAI API error: {e}")
            return None

    # ------------ 其他工具函数 ------------
    def process_data(self, data_list: List[Dict[str, str]]) -> List[Dict[str, str]]:
        return data_list[-self.history_length:]


def main(args=None):
    rclpy.init(args=args)
    node = OllamaChatNode()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()
        rclpy.shutdown()


if __name__ == '__main__':
    main()

import os
from launch import LaunchDescription
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory

def generate_launch_description():
    # TTS 配置文件
    tts_dir = get_package_share_directory('tts')
    tts_config = os.path.join(tts_dir, 'config', 'tts_params.yaml')
    resource_param = {"source_path": tts_dir}

    # 麦克风节点
    wheeltec_mic = Node(
        package="wheeltec_mic_aiui",
        executable="wheeltec_mic",
        output='screen',
        parameters=[{"usart_port_name": "/dev/wheeltec_mic_test",
                    "serial_baud_rate": 115200}]
    )

    # 语音识别节点
    wheeltec_mic_aiui = Node(
        package="wheeltec_mic_aiui",
        executable="wheeltec_mic_aiui",
        output='screen',
    )

    # AI 聊天客户端节点
    chat_service = Node(
        package="wheeltec_mic_aiui",
        executable="chat_service",
        output='screen',
    )

    # TTS 语音合成节点
    tts_node = Node(
        package="tts",
        executable="tts_node",
        output='screen',
        parameters=[resource_param, tts_config]
    )

    ld = LaunchDescription()

    ld.add_action(wheeltec_mic)
    ld.add_action(wheeltec_mic_aiui)
    ld.add_action(chat_service)
    ld.add_action(tts_node)
    
    return ld

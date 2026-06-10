/****************************************************************/
/* Copyright (c) 2023 WHEELTEC Technology, Inc   				*/
/* function:Recording call controller, including sleep function	*/
/* 功能：录音调用控制器，包含休眠功能									*/
/****************************************************************/
#include <signal.h>
#include "call_recognition.h"

using namespace std;
using std::placeholders::_1;

void sighandler(int);

/********************************************************
Function:Wake flag result processing 
功能: 唤醒标志位处理回调函数
*********************************************************/
void Call::awake_flag_Callback(std_msgs::msg::Int8::SharedPtr msg)
{
	int new_awake_flag = msg->data;
	
	// 如果是唤醒状态（从0→1或保持1）
	if (new_awake_flag == 1){
		// 每次唤醒都播报"我在"
		awake_flag = 1;
		recognize_fail_count = 0;
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
		// 播报"我在" - 使用pasuspender暂停PulseAudio（在机械狗本地部署的话把pasuspender注释掉,无界面，直接用aplay播放）
		// string audio_cmd = "pasuspender -- aplay -q -D plughw:2,0 /home/zl/zsibot_L1_ws/src/wheeltec_mic/wheeltec_mic_ros2/feedback_voice/wozai.wav";
		// string audio_cmd = "aplay /home/robot/voice_control_ws/src/wheeltec_mic/wheeltec_mic_ros2/feedback_voice/wozai.wav";
		// string audio_cmd = "aplay -q -D plughw:2,0 /home/zl/zsibot_L1_ws/src/wheeltec_mic/wheeltec_mic_ros2/feedback_voice/wozai.wav";
		//// 通过aplay -l看看用哪个设备来播放声音，直接使用设备卡名，不受卡号的影响，解决每次开关机卡号重分配的问题
		string audio_cmd = "aplay -D plughw:Device,0 /home/guest/offline_chat/src/wheeltec_mic/wheeltec_mic_ros2/feedback_voice/wozai.wav";
		RCLCPP_INFO(this->get_logger(), "🎵 播报：我在");
		system(audio_cmd.c_str());
		
		// 等待播报完成 + 缓冲时间
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
		RCLCPP_INFO(this->get_logger(), "🎤 准备开始语音识别...");
		
		// 每次收到awake_flag=1都触发识别
		call_ = true;
		recognize_fail_count = 0;
	}
	else{
		// 休眠状态
		awake_flag = 0;
		call_ = false;
	}
}

/********************************************************
Function: Service result processing 
功能: 服务结果处理回调函数
*********************************************************/
void Call::server_callback_(rclcpp::Client<wheeltec_mic_msg::srv::GetOfflineResult>::SharedFuture result)
{
	auto response = result.get();

	string str1 = "ok";				//语音识别相关字符串
	string str2 = "fail";						
	string str3 = "小车休眠";					
	string str4 = "失败5次";					
	string str5 = "失败10次";

	if (str3 == response->text){	
		awake_flag=0;
		recognize_fail_count = 0;}
	else if (str1 == response->result){
		recognize_fail_count = 0;
		// 识别成功后不再自动触发下一次识别，等待用户再次唤醒
		RCLCPP_INFO(this->get_logger(), "✅ 指令执行完成，等待下次唤醒...");
		call_ = false;  // 停止自动识别，需要重新唤醒
		awake_flag = 0;  // 重置唤醒标志，要求重新说唤醒词
	}
	else if (str2 == response->result){
		recognize_fail_count++;
		if (recognize_fail_count == 5){
			std_msgs::msg::String count_msg;
			count_msg.data = str4;
			voice_words_pub->publish(count_msg);
		}
		else if (recognize_fail_count == 10){
			std_msgs::msg::String count_msg;
			count_msg.data = str5;
			voice_words_pub->publish(count_msg);
		}
		else if (recognize_fail_count == recognize_fail_count_threshold){
			awake_flag = 0;
			recognize_fail_count = 0;
			std_msgs::msg::String off_msg;
			off_msg.data = str3;
			voice_words_pub->publish(off_msg);
		}
		call_ = true;
	}
	else{
		RCLCPP_INFO(this->get_logger(),"failed to call service \"get_offline_recognise_result_srv\"!");
	}

}

/********************************************************
Function: Client requests service
功能: 客户端请求服务
*********************************************************/
int Call::request_()
{
	while(!Call_Client_->wait_for_service(std::chrono::seconds(2))){
		// while(!rclcpp::ok()){
		// RCLCPP_ERROR(this->get_logger(), "Interrupted while waiting for the service. Exiting.");
		// return 1;
		// }
		RCLCPP_INFO(this->get_logger(), "waiting the service...");
	}
	RCLCPP_INFO(this->get_logger(), "Offline_Recognise_Result Service has found.");
	auto request = std::make_shared<wheeltec_mic_msg::srv::GetOfflineResult::Request>();
	/***离线命令词识别服务参数设置***/
	request->offline_recognise_start = 1;
	request->confidence_threshold = confidence_threshold;
	request->time_per_order = seconds_per_order;
	/***循环频率Hz***/
	rclcpp::Rate loop_rate(10);

	while(rclcpp::ok()){
		if (awake_flag && call_){
			Call_Client_->async_send_request(request,std::bind(&Call::server_callback_,this,_1));
			call_ = false;
		}
		rclcpp::spin_some(this->get_node_base_interface());
		loop_rate.sleep();
	}
	return 0;
}

Call::Call(const std::string &node_name,const rclcpp::NodeOptions &options) 
: rclcpp::Node(node_name,options){
	RCLCPP_INFO(this->get_logger(),"%s node init!\n",node_name.c_str());

	this->declare_parameter<int>("confidence_threshold",20);
	this->declare_parameter<int>("time_per_order",5);

	this->get_parameter("confidence_threshold",confidence_threshold);
	this->get_parameter("time_per_order",seconds_per_order);

	/***唤醒标志位话题订阅者创建***/
	awake_flag_sub = this->create_subscription<std_msgs::msg::Int8>(
		"awake_flag",10,std::bind(&Call::awake_flag_Callback,this,_1));
	/***唤醒标志位话题发布者创建***/
	awake_flag_pub = this->create_publisher<std_msgs::msg::Int8>("awake_flag",10);
	/***离线命令词识别结果话题发布者创建***/
	voice_words_pub = this->create_publisher<std_msgs::msg::String>("voice_words",10);
	/***离线命令词识别服务客服端创建***/
	Call_Client_ = this->create_client<wheeltec_mic_msg::srv::GetOfflineResult>(
		"/get_offline_result_srv");
}

Call::~Call(){
	RCLCPP_INFO(this->get_logger(),"call_recognition_node over!\n");
} 

int main(int argc,char **argv)
{
	rclcpp::init(argc,argv);
	signal(SIGINT, sighandler);
	Call call_node("call_recognition",rclcpp::NodeOptions());
	if (call_node.request_() == 0){
	RCLCPP_INFO(rclcpp::get_logger("call_recognition"),
			"call_recognition done!");
	}
	else{
	RCLCPP_INFO(rclcpp::get_logger("call_recognition"),
			"call_recognition Interrupted!");
	}
	rclcpp::shutdown();
	return 0;
}

void sighandler(int signum)
{
   	printf("capture signal：%d，Interrupt...\n", signum);
   	exit(1);
}
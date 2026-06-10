/************************************************************************************************/
/* Copyright (c) 2023 WHEELTEC Technology, Inc   												*/
/* function:Command controller, command word recognition results into the corresponding action	*/
/* 功能：命令控制器，命令词识别结果转化为对应的执行动作													*/
/************************************************************************************************/
#include "command_recognition.h"
using namespace std;
using std::placeholders::_1;

/**************************************************************************
函数功能：寻找语音开启成功标志位sub回调函数
入口参数：voice_flag_msg  voice_control.cpp
返回  值：无
**************************************************************************/
void Command::voice_flag_Callback(std_msgs::msg::Int8::SharedPtr msg){
	voice_flag = msg->data;
	// WHOLE = head + audio_path + "/voice_control.wav";
	WHOLE = head + audio_path + "/wozai.wav";
	if (voice_flag){
		// 注意：这里不播报，由call_recognition节点在唤醒时播报
		cout<<"语音打开成功"<<endl;
	}
}

/**************************************************************************
函数功能：离线命令词识别结果sub回调函数
入口参数：命令词字符串
返回  值：无
**************************************************************************/
void Command::voice_words_Callback(std_msgs::msg::String::SharedPtr msg){
	/***语音指令***/
	string str1 = msg->data;    //取传入数据
	
	// ==================== 姿态控制命令 ====================
	// 站立命令（站起来/站立/起立）
	string str_stand1 = "站起来", str_stand2 = "站立", str_stand3 = "起立";
	string str_stand_dog1 = "小狗站起来", str_stand_dog2 = "小狗站立", str_stand_dog3 = "小狗起立";
	string str_stand_robot1 = "机械狗站起来", str_stand_robot2 = "机械狗站立", str_stand_robot3 = "机械狗起立";
	string str_stand_doggy1 = "狗子站起来", str_stand_doggy2 = "狗子站立", str_stand_doggy3 = "狗子起立";
	string str_stand_ge1 = "小京站起来", str_stand_ge2 = "小京站立", str_stand_ge3 = "小京起立";
	
	// 趴下命令（趴下/卧倒）
	string str_lie1 = "趴下", str_lie2 = "卧倒";
	string str_lie_dog1 = "小狗趴下", str_lie_dog2 = "小狗卧倒";
	string str_lie_robot1 = "机械狗趴下", str_lie_robot2 = "机械狗卧倒";
	string str_lie_doggy1 = "狗子趴下", str_lie_doggy2 = "狗子卧倒";
	string str_lie_ge1 = "小京趴下", str_lie_ge2 = "小京卧倒";
	
	// 匍匐命令（匍匐/爬行）
	string str_crawl1 = "匍匐", str_crawl2 = "爬行";
	string str_crawl_dog1 = "小狗匍匐", str_crawl_dog2 = "小狗爬行";
	string str_crawl_robot1 = "机械狗匍匐", str_crawl_robot2 = "机械狗爬行";
	string str_crawl_doggy1 = "狗子匍匐", str_crawl_doggy2 = "狗子爬行";
	string str_crawl_ge1 = "小京匍匐", str_crawl_ge2 = "小京爬行";
	
	// ==================== 旋转控制命令 ====================
	// 原地转圈命令（原地转圈/转圈/转一圈/旋转一圈）
	string str_spin1 = "原地转圈", str_spin2 = "转圈", str_spin3 = "转一圈", str_spin4 = "旋转一圈";
	string str_spin_dog1 = "小狗原地转圈", str_spin_dog2 = "小狗转圈", str_spin_dog3 = "小狗转一圈";
	string str_spin_robot1 = "机械狗原地转圈", str_spin_robot2 = "机械狗转圈", str_spin_robot3 = "机械狗转一圈";
	string str_spin_ge1 = "小京原地转圈", str_spin_ge2 = "小京转圈", str_spin_ge3 = "小京转一圈";
	
	// 向后转命令（向后转/后转/掉头/转身）
	string str_turn_back1 = "向后转", str_turn_back2 = "后转", str_turn_back3 = "掉头", str_turn_back4 = "转身";
	string str_turn_back_dog1 = "小狗向后转", str_turn_back_dog2 = "小狗后转", str_turn_back_dog3 = "小狗掉头";
	string str_turn_back_robot1 = "机械狗向后转", str_turn_back_robot2 = "机械狗后转", str_turn_back_robot3 = "机械狗掉头";
	string str_turn_back_ge1 = "小京向后转", str_turn_back_ge2 = "小京后转", str_turn_back_ge3 = "小京掉头";
	
	// 向左转90度命令（向左转/左转90度）
	string str_turn_left_90_1 = "向左转", str_turn_left_90_2 = "左转90度";
	string str_turn_left_90_dog1 = "小狗向左转", str_turn_left_90_dog2 = "小狗左转90度";
	string str_turn_left_90_robot1 = "机械狗向左转", str_turn_left_90_robot2 = "机械狗左转90度";
	string str_turn_left_90_ge1 = "小京向左转", str_turn_left_90_ge2 = "小京左转90度";
	
	// 向右转90度命令（向右转/右转90度）
	string str_turn_right_90_1 = "向右转", str_turn_right_90_2 = "右转90度";
	string str_turn_right_90_dog1 = "小狗向右转", str_turn_right_90_dog2 = "小狗右转90度";
	string str_turn_right_90_robot1 = "机械狗向右转", str_turn_right_90_robot2 = "机械狗右转90度";
	string str_turn_right_90_ge1 = "小京向右转", str_turn_right_90_ge2 = "小京右转90度";
	
	// 左转命令
	string str_left = "左转";
	string str_left_dog = "小狗左转", str_left_robot = "机械狗左转", str_left_ge = "小京左转";
	
	// 右转命令
	string str_right = "右转";
	string str_right_dog = "小狗右转", str_right_robot = "机械狗右转", str_right_ge = "小京右转";
	
	// ==================== 运动控制命令 ====================
	// 前进命令（前进/往前走/向前走/走）
	string str_forward1 = "前进", str_forward2 = "往前走", str_forward3 = "向前走", str_forward4 = "走";
	string str_forward_dog1 = "小狗前进", str_forward_dog2 = "小狗往前走", str_forward_dog3 = "小狗向前走";
	string str_forward_robot1 = "机械狗前进", str_forward_robot2 = "机械狗往前走", str_forward_robot3 = "机械狗向前走";
	string str_forward_ge1 = "小京前进", str_forward_ge2 = "小京往前走", str_forward_ge3 = "小京向前走";
	
	// 后退命令（后退/往后退/向后退/倒退）
	string str_backward1 = "后退", str_backward2 = "往后退", str_backward3 = "向后退", str_backward4 = "倒退";
	string str_backward_dog1 = "小狗后退", str_backward_dog2 = "小狗往后退", str_backward_dog3 = "小狗倒退";
	string str_backward_robot1 = "机械狗后退", str_backward_robot2 = "机械狗往后退", str_backward_robot3 = "机械狗倒退";
	string str_backward_ge1 = "小京后退", str_backward_ge2 = "小京往后退", str_backward_ge3 = "小京倒退";
	
	// 停止命令（停/停止/停下/别动）
	string str_stop1 = "停", str_stop2 = "停止", str_stop3 = "停下", str_stop4 = "别动";
	string str_stop_dog1 = "小狗停", str_stop_dog2 = "小狗停止", str_stop_dog3 = "小狗停下";
	string str_stop_robot1 = "机械狗停", str_stop_robot2 = "机械狗停止", str_stop_robot3 = "机械狗停下";
	string str_stop_ge1 = "小京停", str_stop_ge2 = "小京停止", str_stop_ge3 = "小京停下";
	
	// ==================== 人体追踪命令 ====================
	// 开始跟随命令（跟着我/开始跟随/跟我走/跟上我/跟随我）
	string str_follow1 = "跟着我", str_follow2 = "跟我走", str_follow3 = "跟上我", str_follow4 = "跟随我";
	string str_follow_dog1 = "小狗跟着我", str_follow_dog2 = "小狗跟我走", str_follow_dog3 = "小狗跟随我";
	string str_follow_robot1 = "机械狗跟着我", str_follow_robot2 = "机械狗跟我走", str_follow_robot3 = "机械狗跟随我";
	string str_follow_ge1 = "小京跟着我", str_follow_ge2 = "小京跟我走", str_follow_ge3 = "小京跟随我";
	
	string str_follow_start1 = "开始跟随", str_follow_start2 = "开始跟踪";
	string str_follow_start_dog1 = "小狗开始跟随", str_follow_start_dog2 = "小狗开始跟踪";
	string str_follow_start_robot1 = "机械狗开始跟随", str_follow_start_robot2 = "机械狗开始跟踪";
	string str_follow_start_ge1 = "小京开始跟随", str_follow_start_ge2 = "小京开始跟踪";
	
	// 停止跟随命令（停止跟随/不用跟着我了/别跟了/不要跟了/停止跟踪）
	string str_follow_stop1 = "停止跟随", str_follow_stop2 = "不用跟着我了", str_follow_stop3 = "别跟了";
	string str_follow_stop4 = "不要跟了", str_follow_stop5 = "停止跟踪", str_follow_stop6 = "不用跟了";
	string str_follow_stop_dog1 = "小狗停止跟随", str_follow_stop_dog2 = "小狗不用跟了", str_follow_stop_dog3 = "小狗别跟了";
	string str_follow_stop_robot1 = "机械狗停止跟随", str_follow_stop_robot2 = "机械狗不用跟了", str_follow_stop_robot3 = "机械狗别跟了";
	string str_follow_stop_ge1 = "小京停止跟随", str_follow_stop_ge2 = "小京不用跟着我了", str_follow_stop_ge3 = "小京别跟了", str_follow_stop_ge4 = "小京不用跟了";
	
	// 保留的旧命令（兼容）
	string str12 = "失败5次";
	string str13 = "失败10次";
	string str14 = "遇到障碍物";
	string str15 = "小车唤醒";

	// ========== 姿态控制 ==========
	if (str1 == str_stand1 || str1 == str_stand2 || str1 == str_stand3 ||
	    str1 == str_stand_dog1 || str1 == str_stand_dog2 || str1 == str_stand_dog3 ||
	    str1 == str_stand_robot1 || str1 == str_stand_robot2 || str1 == str_stand_robot3 ||
	    str1 == str_stand_doggy1 || str1 == str_stand_doggy2 || str1 == str_stand_doggy3 ||
	    str1 == str_stand_ge1 || str1 == str_stand_ge2 || str1 == str_stand_ge3){
		publish_posture(1);  // 站立
		cout<<"🟢 好的：站起来"<<endl;
	std::this_thread::sleep_for(std::chrono::milliseconds(500));  // 等待录音设备完全释放
		WHOLE = head + audio_path + "/OK.wav";
		system(WHOLE.c_str());  // 播报语音反馈
		std::this_thread::sleep_for(std::chrono::milliseconds(500));  // 等待播报完成
	}
	else if (str1 == str_lie1 || str1 == str_lie2 ||
	         str1 == str_lie_dog1 || str1 == str_lie_dog2 ||
	         str1 == str_lie_robot1 || str1 == str_lie_robot2 ||
	         str1 == str_lie_doggy1 || str1 == str_lie_doggy2 ||
	         str1 == str_lie_ge1 || str1 == str_lie_ge2){
		publish_posture(2);  // 趴下
		cout<<"🟡 好的：趴下"<<endl;
	std::this_thread::sleep_for(std::chrono::milliseconds(500));  // 等待录音设备完全释放
		WHOLE = head + audio_path + "/OK.wav";
		system(WHOLE.c_str());  // 播报语音反馈
		std::this_thread::sleep_for(std::chrono::milliseconds(500));  // 等待播报完成
	}
	else if (str1 == str_crawl1 || str1 == str_crawl2 ||
	         str1 == str_crawl_dog1 || str1 == str_crawl_dog2 ||
	         str1 == str_crawl_robot1 || str1 == str_crawl_robot2 ||
	         str1 == str_crawl_doggy1 || str1 == str_crawl_doggy2 ||
	         str1 == str_crawl_ge1 || str1 == str_crawl_ge2){
		publish_posture(3);  // 匍匐
		cout<<"🟠 好的：匍匐"<<endl;
	std::this_thread::sleep_for(std::chrono::milliseconds(500));  // 等待录音设备完全释放
	WHOLE = head + audio_path + "/OK.wav";
	system(WHOLE.c_str());  // 播报语音反馈（阻塞直到播放完成）
}

	// ========== 旋转控制 ==========
	else if (str1 == str_spin1 || str1 == str_spin2 || str1 == str_spin3 || str1 == str_spin4 ||
	         str1 == str_spin_dog1 || str1 == str_spin_dog2 || str1 == str_spin_dog3 ||
	         str1 == str_spin_robot1 || str1 == str_spin_robot2 || str1 == str_spin_robot3 ||
	         str1 == str_spin_ge1 || str1 == str_spin_ge2 || str1 == str_spin_ge3){
		cout<<"🔄 好的：原地转圈"<<endl;
	std::this_thread::sleep_for(std::chrono::milliseconds(500));  // 等待录音设备完全释放
	WHOLE = head + audio_path + "/OK.wav";
	system(WHOLE.c_str());  // 播报语音反馈（阻塞直到播放完成）
		rotate_angle(240.0, 0.5);  // 原地转圈，调整为240度实现接近一圈的旋转
	}
	else if (str1 == str_turn_back1 || str1 == str_turn_back2 || str1 == str_turn_back3 || str1 == str_turn_back4 ||
	         str1 == str_turn_back_dog1 || str1 == str_turn_back_dog2 || str1 == str_turn_back_dog3 ||
	         str1 == str_turn_back_robot1 || str1 == str_turn_back_robot2 || str1 == str_turn_back_robot3 ||
	         str1 == str_turn_back_ge1 || str1 == str_turn_back_ge2 || str1 == str_turn_back_ge3){
		cout<<"↩️ 好的：向后转"<<endl;
	std::this_thread::sleep_for(std::chrono::milliseconds(500));  // 等待录音设备完全释放
	WHOLE = head + audio_path + "/OK.wav";
	system(WHOLE.c_str());  // 播报语音反馈（阻塞直到播放完成）
		rotate_angle(120.0, 0.5);  // 向后转，调整为120度实现接近180度的旋转
	}
	else if (str1 == str_turn_left_90_1 || str1 == str_turn_left_90_2 ||
	         str1 == str_turn_left_90_dog1 || str1 == str_turn_left_90_dog2 ||
	         str1 == str_turn_left_90_robot1 || str1 == str_turn_left_90_robot2 ||
	         str1 == str_turn_left_90_ge1 || str1 == str_turn_left_90_ge2){
		cout<<"↰ 好的：向左转90度"<<endl;
	std::this_thread::sleep_for(std::chrono::milliseconds(500));  // 等待录音设备完全释放
	WHOLE = head + audio_path + "/OK.wav";
	system(WHOLE.c_str());  // 播报语音反馈（阻塞直到播放完成）
		rotate_angle(60.0, 0.5);  // 向左转，调整为60度实现接近90度的旋转
	}
	else if (str1 == str_turn_right_90_1 || str1 == str_turn_right_90_2 ||
	         str1 == str_turn_right_90_dog1 || str1 == str_turn_right_90_dog2 ||
	         str1 == str_turn_right_90_robot1 || str1 == str_turn_right_90_robot2 ||
	         str1 == str_turn_right_90_ge1 || str1 == str_turn_right_90_ge2){
		cout<<"↱ 好的：向右转90度"<<endl;
	std::this_thread::sleep_for(std::chrono::milliseconds(500));  // 等待录音设备完全释放
	WHOLE = head + audio_path + "/OK.wav";
	system(WHOLE.c_str());  // 播报语音反馈（阻塞直到播放完成）
		rotate_angle(-60.0, 0.5);  // 向右转，调整为-60度实现接近-90度的旋转
	}
	else if (str1 == str_left || str1 == str_left_dog || str1 == str_left_robot || str1 == str_left_ge){
		cout<<"← 好的：左转"<<endl;
	std::this_thread::sleep_for(std::chrono::milliseconds(500));  // 等待录音设备完全释放
	WHOLE = head + audio_path + "/OK.wav";
	system(WHOLE.c_str());  // 播报语音反馈（阻塞直到播放完成）
		publish_velocity(0.0, 0.5, 1.0);  // 左转1秒
	}
	else if (str1 == str_right || str1 == str_right_dog || str1 == str_right_robot || str1 == str_right_ge){
		cout<<"→ 好的：右转"<<endl;
	std::this_thread::sleep_for(std::chrono::milliseconds(500));  // 等待录音设备完全释放
	WHOLE = head + audio_path + "/OK.wav";
	system(WHOLE.c_str());  // 播报语音反馈（阻塞直到播放完成）
		publish_velocity(0.0, -0.5, 1.0);  // 右转1秒
	}
	
	// ========== 运动控制 ==========
	else if (str1 == str_forward1 || str1 == str_forward2 || str1 == str_forward3 || str1 == str_forward4 ||
	         str1 == str_forward_dog1 || str1 == str_forward_dog2 || str1 == str_forward_dog3 ||
	         str1 == str_forward_robot1 || str1 == str_forward_robot2 || str1 == str_forward_robot3 ||
	         str1 == str_forward_ge1 || str1 == str_forward_ge2 || str1 == str_forward_ge3){
		cout<<"↑ 好的：前进"<<endl;
	std::this_thread::sleep_for(std::chrono::milliseconds(500));  // 等待录音设备完全释放
	// WHOLE = head + audio_path + "/car_front.wav";
	WHOLE = head + audio_path + "/OK.wav";
	system(WHOLE.c_str());  // 播报语音反馈（阻塞直到播放完成）
		publish_velocity(0.3, 0.0, 2.0);  // 前进2秒
	}
	else if (str1 == str_backward1 || str1 == str_backward2 || str1 == str_backward3 || str1 == str_backward4 ||
	         str1 == str_backward_dog1 || str1 == str_backward_dog2 || str1 == str_backward_dog3 ||
	         str1 == str_backward_robot1 || str1 == str_backward_robot2 || str1 == str_backward_robot3 ||
	         str1 == str_backward_ge1 || str1 == str_backward_ge2 || str1 == str_backward_ge3){
		cout<<"↓ 好的：后退"<<endl;
	std::this_thread::sleep_for(std::chrono::milliseconds(500));  // 等待录音设备完全释放
	WHOLE = head + audio_path + "/OK.wav";
	system(WHOLE.c_str());  // 播报语音反馈（阻塞直到播放完成）
		publish_velocity(-0.3, 0.0, 2.0);  // 后退2秒
	}
	else if (str1 == str_stop1 || str1 == str_stop2 || str1 == str_stop3 || str1 == str_stop4 ||
	         str1 == str_stop_dog1 || str1 == str_stop_dog2 || str1 == str_stop_dog3 ||
	         str1 == str_stop_robot1 || str1 == str_stop_robot2 || str1 == str_stop_robot3 ||
	         str1 == str_stop_ge1 || str1 == str_stop_ge2 || str1 == str_stop_ge3){
		cout<<"⏹️ 好的：停止"<<endl;
	std::this_thread::sleep_for(std::chrono::milliseconds(500));  // 等待录音设备完全释放
	WHOLE = head + audio_path + "/OK.wav";
	system(WHOLE.c_str());  // 播报语音反馈（阻塞直到播放完成）
		publish_velocity(0.0, 0.0);  // 停止
	}
	
	// ========== 人体追踪控制 ==========
	else if (str1 == str_follow1 || str1 == str_follow2 || str1 == str_follow3 || str1 == str_follow4 ||
	         str1 == str_follow_dog1 || str1 == str_follow_dog2 || str1 == str_follow_dog3 ||
	         str1 == str_follow_robot1 || str1 == str_follow_robot2 || str1 == str_follow_robot3 ||
	         str1 == str_follow_ge1 || str1 == str_follow_ge2 || str1 == str_follow_ge3 ||
	         str1 == str_follow_start1 || str1 == str_follow_start2 ||
	         str1 == str_follow_start_dog1 || str1 == str_follow_start_dog2 ||
	         str1 == str_follow_start_robot1 || str1 == str_follow_start_robot2 ||
	         str1 == str_follow_start_ge1 || str1 == str_follow_start_ge2){
		// 智能选择跟随目标：基于声源方向选择最近的人体
		int selected_id = select_target_by_voice_direction();
		
		if (selected_id >= 0){
			set_target_track_id(selected_id);  // 通过参数设置目标ID
			cout<<"👤 好的：开始跟随 (智能选择ID=" << selected_id << ")"<<endl;
		std::this_thread::sleep_for(std::chrono::milliseconds(500));  // 等待录音设备完全释放
			WHOLE = head + audio_path + "/OK.wav";
			system(WHOLE.c_str());  // 播报语音反馈
			std::this_thread::sleep_for(std::chrono::milliseconds(500));  // 等待播报完成
		}
		else{
			cout<<"⚠️  未检测到人体，请稍后再试"<<endl;
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
		}
	}
	else if (str1 == str_follow_stop1 || str1 == str_follow_stop2 || str1 == str_follow_stop3 ||
	         str1 == str_follow_stop4 || str1 == str_follow_stop5 || str1 == str_follow_stop6 ||
	         str1 == str_follow_stop_dog1 || str1 == str_follow_stop_dog2 || str1 == str_follow_stop_dog3 ||
	         str1 == str_follow_stop_robot1 || str1 == str_follow_stop_robot2 || str1 == str_follow_stop_robot3 ||
	         str1 == str_follow_stop_ge1 || str1 == str_follow_stop_ge2 || str1 == str_follow_stop_ge3 || str1 == str_follow_stop_ge4){
		// 停止人体追踪：设置target_track_id为-1
		set_target_track_id(-1);  // 通过参数设置为-1表示停止
		publish_velocity(0.0, 0.0);  // 同时停止运动
		cout<<"🛑 好的：停止跟随"<<endl;
	std::this_thread::sleep_for(std::chrono::milliseconds(500));  // 等待录音设备完全释放
		WHOLE = head + audio_path + "/OK.wav";
		system(WHOLE.c_str());  // 播报语音反馈
		std::this_thread::sleep_for(std::chrono::milliseconds(500));  // 等待播报完成
	}
	else if (str1 == str12){
		cout<<"您已经连续【输入空指令or识别失败】5次，累计达15次自动进入休眠，输入有效指令后计数清零"<<endl;
	}
	else if (str1 == str13){
		cout<<"您已经连续【输入空指令or识别失败】10次，累计达15次自动进入休眠，输入有效指令后计数清零"<<endl;
	}
	else if (str1 == str14){
		WHOLE = head + audio_path + "/Tracker.wav";
		system(WHOLE.c_str());  // 播报语音反馈
		cout<<"小车遇到障碍物，已停止运动"<<endl;
	}
	else if (str1 == str15){
		WHOLE = head + audio_path + "/awake.wav";
		system(WHOLE.c_str());  // 播报语音反馈
		cout<<"小车已被唤醒，请说语音指令"<<endl;
	}
	// ========== 保留的旧功能（如需使用可保留） ==========
}

Command::Command(const std::string &node_name)
: rclcpp::Node(node_name){
	RCLCPP_INFO(this->get_logger(),"%s node init!\n",node_name.c_str());
	/***声明参数并获取***/
	this->declare_parameter<string>("audio_path","");
	this->get_parameter("audio_path",audio_path);
	/***唤醒标志位话题发布者创建***/
	awake_flag_pub = this->create_publisher<std_msgs::msg::Int8>("awake_flag",10); 
	/***自主建图标志位话题发布者创建***/
	rrt_flag_pub = this->create_publisher<std_msgs::msg::Int8>("rrt_flag",10); 
	/***雷达跟随标志位话题发布者创建***/
	laser_follow_flag_pub = this->create_publisher<std_msgs::msg::Int8>("laser_follow_flag",10); 
	/***色块跟随标志位话题发布者创建***/
	visual_follow_flag_pub = this->create_publisher<std_msgs::msg::Int8>("visual_follow_flag",10);
	
	/***机械狗控制发布者创建***/
	posture_pub = this->create_publisher<std_msgs::msg::Int32>("/robot/posture_command",10);
	cmd_vel_pub = this->create_publisher<geometry_msgs::msg::Twist>("/cmd_vel",10);
	
	/***参数客户端创建（用于设置yolov8_bytetrack_node的参数）***/
	param_client = std::make_shared<rclcpp::AsyncParametersClient>(this, "/yolov8_bytetrack_node");
	
	/***离线命令词识别结果话题订阅者创建***/
	voice_words_sub = this->create_subscription<std_msgs::msg::String>(
		"voice_words",10,std::bind(&Command::voice_words_Callback,this,_1));
	/***寻找语音开启标志位话题订阅者创建***/
	voice_flag_sub = this->create_subscription<std_msgs::msg::Int8>(
		"voice_flag",10,std::bind(&Command::voice_flag_Callback,this,_1));	/***声源方向角度话题订阅者创建***/
	awake_angle_sub = this->create_subscription<std_msgs::msg::UInt32>(
		"awake_angle",10,std::bind(&Command::awake_angle_Callback,this,_1));
	/***人体检测信息话题订阅者创建***/
	detections_sub = this->create_subscription<std_msgs::msg::String>(
		"/person/detections_info",10,std::bind(&Command::detections_Callback,this,_1));
	sleep(2);
	cout<<"========================================"<<endl;
	cout<<"🐕 机械狗语音控制已启动！"<<endl;
	cout<<"========================================"<<endl;
	cout<<"🟢 姿态控制:"<<endl;
	cout<<"  小京站起来/站立/起立 ———> 站立姿态"<<endl;
	cout<<"  小京趴下/卧倒 ———> 趴下姿态"<<endl;
	cout<<"  小京匍匐/爬行 ———> 匍匐姿态"<<endl;
	cout<<"\n🔄 旋转控制:"<<endl;
	cout<<"  小京原地转圈/转圈/转一圈 ———> 360度旋转"<<endl;
	cout<<"  小京向后转/后转/掉头 ———> 180度转身"<<endl;
	cout<<"  小京向左转/左转90度 ———> 左转90度"<<endl;
	cout<<"  小京向右转/右转90度 ———> 右转90度"<<endl;
	cout<<"  小京左转 ———> 向左转"<<endl;
	cout<<"  小京右转 ———> 向右转"<<endl;
	cout<<"\n⬆️ 运动控制:"<<endl;
	cout<<"  小京前进/往前走/向前走 ———> 向前移动"<<endl;
	cout<<"  小京后退/往后退/倒退 ———> 向后移动"<<endl;
	cout<<"  小京停/停止/停下 ———> 停止运动"<<endl;
	cout<<"\n👤 人体追踪:"<<endl;
	cout<<"  小京跟着我/跟我走/跟随我 ———> 启动人体跟随"<<endl;
	cout<<"  小京不用跟着我了/别跟了 ———> 停止人体跟随"<<endl;
	cout<<"\n💡 提示：所有命令也支持'小狗'/'机械狗'/'狗子'前缀"<<endl;
	cout<<"========================================"<<endl;
}

void Command::run(){
	while(rclcpp::ok()){
		rclcpp::spin_some(this->get_node_base_interface());
		}
}

Command::~Command(){
	RCLCPP_INFO(this->get_logger(),"command_recognition_node over!\n");
}

/**************************************************************************
函数功能：发布姿态控制命令
入口参数：posture_cmd - 姿态命令 (1=站立, 2=趴下, 3=匍匐)
返回  值：无
**************************************************************************/
void Command::publish_posture(int posture_cmd){
	std_msgs::msg::Int32 msg;
	msg.data = posture_cmd;
	posture_pub->publish(msg);
}

/**************************************************************************
函数功能：发布速度控制命令
入口参数：linear_x - 线速度(m/s), angular_z - 角速度(rad/s), duration - 持续时间(秒,0表示持续发送)
返回  值：无
**************************************************************************/
void Command::publish_velocity(double linear_x, double angular_z, double duration){
	geometry_msgs::msg::Twist msg;
	msg.linear.x = linear_x;
	msg.linear.y = 0.0;
	msg.linear.z = 0.0;
	msg.angular.x = 0.0;
	msg.angular.y = 0.0;
	msg.angular.z = angular_z;
	
	if (duration > 0.0){
		// 持续发送指定时间
		auto start_time = std::chrono::steady_clock::now();
		while (rclcpp::ok()){
			cmd_vel_pub->publish(msg);
			
			auto now = std::chrono::steady_clock::now();
			auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time).count();
			if (elapsed >= duration * 1000){
				break;
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
		}
		// 停止运动
		msg.linear.x = 0.0;
		msg.angular.z = 0.0;
		cmd_vel_pub->publish(msg);
	}
	else{
		// 单次发送
		cmd_vel_pub->publish(msg);
	}
}

/**************************************************************************
函数功能：旋转指定角度
入口参数：angle_deg - 旋转角度(度), angular_speed - 角速度(rad/s)
返回  值：无
**************************************************************************/
void Command::rotate_angle(double angle_deg, double angular_speed){
	// 转换为弧度
	double angle_rad = angle_deg * M_PI / 180.0;
	// 计算所需时间
	double duration = fabs(angle_rad / angular_speed);
	// 根据角度符号确定旋转方向
	double angular_z = (angle_deg > 0) ? angular_speed : -angular_speed;
	
	publish_velocity(0.0, angular_z, duration);
}

/**************************************************************************
函数功能：声源方向角度回调
入口参数：msg - 声源角度消息(度)
返回  值：无
**************************************************************************/
void Command::awake_angle_Callback(std_msgs::msg::UInt32::SharedPtr msg){
	awake_angle = msg->data;
	RCLCPP_DEBUG(this->get_logger(), "收到声源方向: %u°", awake_angle);
}

/**************************************************************************
函数功能：人体检测信息回调
入口参数：msg - JSON格式的检测信息
返回  值：无
**************************************************************************/
void Command::detections_Callback(std_msgs::msg::String::SharedPtr msg){
	detections_json = msg->data;
	RCLCPP_DEBUG(this->get_logger(), "收到检测信息");
}

/**************************************************************************
函数功能：基于声源方向智能选择跟随目标
算法逻辑：
  1. 将声源角度转换为相机坐标系角度（[-180, 180]）
  2. 遍历所有检测到的人体，计算每个人相对机器人的角度
  3. 使用最短角度差（处理环绕问题）选择最接近声源方向且距离最近的人体
返回  值：选中的track_id，-1表示未找到合适目标
**************************************************************************/
int Command::select_target_by_voice_direction(){
	if (detections_json.empty()){
		RCLCPP_WARN(this->get_logger(), "没有可用的检测信息");
		return -1;
	}
	
	try{
		// 解析JSON
		auto detections = json::parse(detections_json);
		if (detections.empty()){
			RCLCPP_WARN(this->get_logger(), "未检测到任何人体");
			return -1;
		}
		
		// 声源角度转换：麦克风阵列0°=正前方，顺时针0-360°
		// 转换为标准角度系统[-180°, 180°]：
		//   - 0°保持为0°（正前方）
		//   - 350°变为-10°（左前方）
		//   - 10°保持为10°（右前方）
		//   - 180°保持为180°（正后方）
		double voice_angle_centered = static_cast<double>(awake_angle);
		if (voice_angle_centered > 180.0) {
			voice_angle_centered -= 360.0;  // 将[180, 360]映射到[-180, 0]
		}
		
		RCLCPP_INFO(this->get_logger(), 
			"🎤 声源方向: %u° → 标准角度: %.1f°", 
			awake_angle, voice_angle_centered);
		
		// 相机参数（L1相机：1280x720，HFOV=111°）
		const double HFOV = 111.0;  // 水平视场角（度）
		const double IMG_WIDTH = 1280.0;  // 图像宽度（像素）
		
		// 辅助函数：计算两个角度之间的最短角度差（处理环绕）
		auto normalize_angle_diff = [](double diff) -> double {
			// 将角度差限制在[-180, 180]范围内
			while (diff > 180.0) diff -= 360.0;
			while (diff < -180.0) diff += 360.0;
			return std::abs(diff);
		};
		
		// 选择最佳目标
		int best_id = -1;
		double best_score = std::numeric_limits<double>::max();
		double best_angle_diff = 180.0;
		
		for (const auto& det : detections){
			int track_id = det["track_id"];
			double center_x = det["center"][0];
			double distance = det["distance"];
			
			// 计算人体在相机视野中的角度（相对机器人正前方，左正右负）
			double alpha_rad = ((center_x - IMG_WIDTH / 2.0) / IMG_WIDTH) * (HFOV * M_PI / 180.0);
			double person_angle = alpha_rad * 180.0 / M_PI;  // 转换为度，范围约[-55.5°, +55.5°]
			
			// 计算角度差异（使用最短路径，处理环绕问题）
			double angle_diff = normalize_angle_diff(person_angle - voice_angle_centered);
			
			// 评分策略：
			// - 角度差异权重80%（声源方向是主要依据）
			// - 距离权重20%（距离为次要因素）
			// - 归一化：角度范围[0, 180]，距离假设范围[0, 5]
			double score = (angle_diff / 180.0) * 0.8 + (distance / 5.0) * 0.2;
			
			RCLCPP_INFO(this->get_logger(),
				"  候选 ID=%d: 相机角度=%.1f°, 距离=%.2fm, 角度差=%.1f°, 评分=%.3f",
				track_id, person_angle, distance, angle_diff, score);
			
			// 筛选条件：角度差小于60°（声源方向前方±60°扇区内）
			if (angle_diff < 60.0 && score < best_score){
				best_score = score;
				best_id = track_id;
				best_angle_diff = angle_diff;
			}
		}
		
		if (best_id >= 0){
			RCLCPP_INFO(this->get_logger(), 
				"✅ 智能选择目标: ID=%d (角度差=%.1f°, 评分=%.3f)", 
				best_id, best_angle_diff, best_score);
		}
		else{
			RCLCPP_WARN(this->get_logger(), 
				"⚠️  未找到声源方向±60°范围内的人体");
		}
		
		return best_id;
	}
	catch(const json::exception& e){
		RCLCPP_ERROR(this->get_logger(), "JSON解析错误: %s", e.what());
		return -1;
	}
}
/**************************************************************************
函数功能：设置人体跟踪目标ID（通过参数）
入口参数：track_id - 目标跟踪ID（-1表示停止跟随）
返回  值：无
**************************************************************************/
void Command::set_target_track_id(int track_id){
	if (!param_client->wait_for_service(std::chrono::seconds(1))){
		RCLCPP_ERROR(this->get_logger(), "参数服务不可用，请确保yolov8_bytetrack_node正在运行");
		return;
	}
	
	// 构建参数
	auto param = rclcpp::Parameter("target_track_id", track_id);
	
	// 异步设置参数（不等待结果，避免阻塞）
	auto result_future = param_client->set_parameters({param});
	
	// 使用 wait_for 等待结果，避免使用 spin_until_future_complete 导致节点重复添加到 executor
	auto status = result_future.wait_for(std::chrono::seconds(1));
	
	if (status == std::future_status::ready){
		try{
			auto result = result_future.get();
			if (!result.empty() && result[0].successful){
				RCLCPP_INFO(this->get_logger(), "✓ 已设置target_track_id=%d", track_id);
			}
			else{
				RCLCPP_ERROR(this->get_logger(), "设置参数失败: %s", 
					result.empty() ? "未知错误" : result[0].reason.c_str());
			}
		}
		catch(const std::exception& e){
			RCLCPP_ERROR(this->get_logger(), "获取参数设置结果异常: %s", e.what());
		}
	}
	else if (status == std::future_status::timeout){
		RCLCPP_WARN(this->get_logger(), "设置参数超时");
	}
	else{
		RCLCPP_WARN(this->get_logger(), "设置参数延迟执行");
	}
}
int main(int argc, char *argv[])
{
	rclcpp::init(argc,argv);
	Command command("command_recognition");
	command.run();
	rclcpp::shutdown();
	return 0;
}
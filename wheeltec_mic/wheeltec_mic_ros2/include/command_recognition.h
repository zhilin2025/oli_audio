#ifndef __CALL_COMMAND_RECOGNITION_H_
#define __CALL_COMMAND_RECOGNITION_H_

#include <iostream>
#include <unistd.h>
#include <play_path.h>
#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/string.hpp>
#include <std_msgs/msg/int8.hpp>
#include <std_msgs/msg/int32.hpp>
#include <std_msgs/msg/u_int32.hpp>
#include <geometry_msgs/msg/twist.hpp>
#include <nlohmann/json.hpp>
#include <cmath>
#include <vector>
using namespace std;
using json = nlohmann::json;

class Command : public rclcpp::Node{
public:
	Command(const std::string &node_name);
	~Command();
	void run();

private:
	int voice_flag = 0;
	
	// 智能跟随相关变量
	uint32_t awake_angle = 0;  // 声源方向角度(度)
	std::string detections_json = "";  // 最新的检测信息JSON

	rclcpp::Publisher<std_msgs::msg::Int8>::SharedPtr rrt_flag_pub;
	rclcpp::Publisher<std_msgs::msg::Int8>::SharedPtr awake_flag_pub;
	rclcpp::Publisher<std_msgs::msg::Int8>::SharedPtr laser_follow_flag_pub;
	rclcpp::Publisher<std_msgs::msg::Int8>::SharedPtr visual_follow_flag_pub;
	// 机械狗控制发布者
	rclcpp::Publisher<std_msgs::msg::Int32>::SharedPtr posture_pub;
	rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr cmd_vel_pub;
	
	// 参数设置客户端（用于设置yolov8_bytetrack_node的target_track_id参数）
	rclcpp::AsyncParametersClient::SharedPtr param_client;
	
    rclcpp::Subscription<std_msgs::msg::Int8>::SharedPtr voice_flag_sub;
    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr voice_words_sub;
	rclcpp::Subscription<std_msgs::msg::UInt32>::SharedPtr awake_angle_sub;
	rclcpp::Subscription<std_msgs::msg::String>::SharedPtr detections_sub;

    void voice_flag_Callback(const std_msgs::msg::Int8::SharedPtr msg);
    void voice_words_Callback(const std_msgs::msg::String::SharedPtr msg);
	void awake_angle_Callback(const std_msgs::msg::UInt32::SharedPtr msg);
	void detections_Callback(const std_msgs::msg::String::SharedPtr msg);
	
	// 辅助函数
	void publish_posture(int posture_cmd);
	void publish_velocity(double linear_x, double angular_z, double duration = 0.0);
	void rotate_angle(double angle_deg, double angular_speed = 0.5);
	int select_target_by_voice_direction();  // 智能选择跟随目标
	void set_target_track_id(int track_id);  // 设置跟踪目标ID参数
};

#endif

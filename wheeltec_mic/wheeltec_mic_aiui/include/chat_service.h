#ifndef CHAT_H_
#define CHAT_H_

#include <iostream>
#include <chrono>
#include <random>
#include <thread>
#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/string.hpp>
#include "ollama_ros_msgs/srv/chat.hpp"
#include <ament_index_cpp/get_package_share_directory.hpp>
#include <vector>

class  Chat_Node : public rclcpp::Node{
public:
    Chat_Node(const std::string &node_name,
        const rclcpp::NodeOptions &options);
    ~Chat_Node();
    void sendMessage(const std::string& message);
    void voice_words_Callback(const std_msgs::msg::String::SharedPtr msg);
    void response_callback(rclcpp::Client<ollama_ros_msgs::srv::Chat>::SharedFuture future);

private:
    std::string tts_text;
    bool waiting_for_response_ = false;
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr chat_words_pub;
    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr voice_words_sub;
    
    rclcpp::Client<ollama_ros_msgs::srv::Chat>::SharedPtr client_;
    std::string removeTags(const std::string& input);

    std::string trim(const std::string& str);
    void playAudio(const std::string& audio_file);
    void shakehand();
    const std::vector<std::string> hand_keywords = {"握手", "握个手"};

    void playThinkingAudio();  // 播放思考语音的函数
};

#endif /* CHAT_H_ */
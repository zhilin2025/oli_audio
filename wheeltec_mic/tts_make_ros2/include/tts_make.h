#ifndef __TTS_MAKE_H_
#define __TTS_MAKE_H_

#include <time.h>
#include <thread>
#include <vector>
#include "rclcpp/rclcpp.hpp"
#include <std_msgs/msg/string.hpp>
#include <std_msgs/msg/int8.hpp>
#include <std_msgs/msg/int32.hpp>

using namespace std;

class TTS : public rclcpp::Node{
public:
    TTS(const std::string &node_name,
         const rclcpp::NodeOptions &options);
    ~TTS();

    int init();
    void feedback_words_callback(const std_msgs::msg::String::SharedPtr msg);

private:
    string source_path;      // 资源文件路径（不变）
    string audio_output_path;  // 音频输出路径
    string appid;
    string voice_name;
    string tts_text;
    int rdn;
    int volume;
    int pitch;
    int speed;
    int sample_rate;
    char* result;
    const char* params_l;
    const char* params_s;
    const char* params_f;
    const char* params_t;
    
    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr feedback_words_sub;
    bool msp_logged_in;

    string current_time();
    string ws2s(const std::wstring &wstr);
    wstring s2ws(const std::string &str);
    string clean_text_for_tts(const string& text);

    int text_to_speech(const char* src_text, const char* des_path, const char* params);
    int synthesize_text(const string& text);
};

#endif
#include "chat_service.h"
#include <algorithm>  // 用于trim函数

/**************************************************************************
函数功能：去除字符串首尾的空白字符（空格、制表符、换行、回车）
**************************************************************************/
std::string Chat_Node::trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\n\r");
    if (start == std::string::npos) { // 全是空白字符
        return "";
    }
    size_t end = str.find_last_not_of(" \t\n\r");
    return str.substr(start, end - start + 1);
}

/**************************************************************************
函数功能：播放指定路径的音频文件（后台线程执行，避免阻塞）
**************************************************************************/
void Chat_Node::playAudio(const std::string& audio_file) {
    try {
        // 后台线程播放音频，避免阻塞主线程
        std::thread([audio_file, this]() {
            std::string play_cmd = "aplay -D plughw:CARD=Device,DEV=0 " + audio_file;
            int result = system(play_cmd.c_str());
            if (result != 0) {
                RCLCPP_WARN(this->get_logger(), "音频播放失败(设备可能被占用): %s", audio_file.c_str());
            }
        }).detach();
    } catch (const std::exception& e) {
        RCLCPP_WARN(this->get_logger(), "无法播放音频 [%s]，错误：%s", audio_file.c_str(), e.what());
    }
}

/**************************************************************************
函数功能：识别结果sub回调函数
**************************************************************************/
void Chat_Node::voice_words_Callback(const std_msgs::msg::String::SharedPtr msg){
    std::string chat_text = msg->data;    //取传入数据
    RCLCPP_INFO(this->get_logger(), "voice_words_callback,监听初始语音数据: %s", chat_text.c_str());
    
    // 1. 去除首尾空白字符，判断是否为空
    std::string trimmed_text = trim(chat_text);
    if (trimmed_text.empty()) {
        RCLCPP_INFO(this->get_logger(), "识别结果为空，不发送给AI，播放无识别结果提示音");
        // 2. 播放指定的空结果提示音
        std::string no_words_audio = "/home/guest/offline_chat/src/wheeltec_mic/wheeltec_mic_aiui/AIUI/audio/noword.wav";
        playAudio(no_words_audio);
        return; // 终止后续逻辑，不调用AI
    }
    
    bool trigger_hand = false;
    // 遍历关键词，判断当前文本是否命中
    for (const auto& kw : hand_keywords)
    {
        if (trimmed_text.find(kw) != std::string::npos)
        {
            trigger_hand = true;
            break;
        }
    }

    if (trigger_hand)
    {
        RCLCPP_INFO(this->get_logger(), ">>>> 识别到握手指令，拦截AI交互");
        // 1. 播放握手音频 shakehand.wav
        std::string hand_audio = "/home/guest/offline_chat/src/wheeltec_mic/wheeltec_mic_aiui/AIUI/audio/shakehand.wav";
        playAudio(hand_audio);
        // 2. 执行握手动作函数
        shakehand();
        // 关键：直接return，不再执行sendMessage，不转发给AI
        return;
    }
    
    // 3. 识别结果有效，正常发送给AI处理
    sendMessage(chat_text);
}

void Chat_Node::shakehand()
{
    std::thread([]()
    {
        system("python3 src/wheeltec_mic/test.py");
    }).detach();
}

/**************************************************************************
函数功能：对话服务请求发送
**************************************************************************/
void Chat_Node::sendMessage(const std::string& message) {
    auto request = std::make_shared<ollama_ros_msgs::srv::Chat::Request>();
    request->content = message;
    waiting_for_response_ = true;
    
    // 播放思考提示音
    playThinkingAudio();
    
    // 使用回调处理响应，不保存 future
    client_->async_send_request(
        request,
        [this](rclcpp::Client<ollama_ros_msgs::srv::Chat>::SharedFuture future) {
            this->response_callback(future);
        }
    );
}

/**************************************************************************
函数功能：对话服务response处理
**************************************************************************/
void Chat_Node::response_callback(rclcpp::Client<ollama_ros_msgs::srv::Chat>::SharedFuture future) {
    try {
        auto response = future.get();
        if (response) {
            std::string rmText = removeTags(response->content);
            std::cout << rmText << std::endl;
            std_msgs::msg::String result_text;
            result_text.data = rmText;
            chat_words_pub->publish(result_text);
            waiting_for_response_ = false;
        }
    } catch (const std::exception& e) {
        RCLCPP_ERROR(this->get_logger(), "Error processing response: %s", e.what());
    }
}

/**************************************************************************
函数功能：移除think标签
**************************************************************************/
std::string Chat_Node::removeTags(const std::string& input) {
    std::string result = input;
    size_t start_pos = 0;

    // 循环查找并移除 <think> 和 </think> 及其之间的内容
    while ((start_pos = result.find("<think>", start_pos)) != std::string::npos) {
        size_t end_pos = result.find("</think>", start_pos);
        if (end_pos == std::string::npos) {
            // 如果没有找到对应的结束标签，直接返回结果
            break;
        }
        // 计算需要移除的部分长度
        size_t length_to_remove = end_pos - start_pos + strlen("</think>");
        result.erase(start_pos, length_to_remove);
        // 更新搜索起点
        start_pos = start_pos; 
    }
    return result;
}

Chat_Node::Chat_Node(const std::string &node_name,
    const rclcpp::NodeOptions &options) : rclcpp::Node(node_name, options){
    RCLCPP_INFO(this->get_logger(),"%s node init!\n",node_name.c_str());

    /***服务客户端创建***/
    client_ = this->create_client<ollama_ros_msgs::srv::Chat>("chat_service");
    /***对话文本话题发布者创建***/
    chat_words_pub = this->create_publisher<std_msgs::msg::String>("feedback_words",10);
    /***识别结果话题订阅者创建***/
    voice_words_sub = this->create_subscription<std_msgs::msg::String>(
        "voice_words",10,std::bind(&Chat_Node::voice_words_Callback,this,std::placeholders::_1));

    // 等待服务可用
    while (!client_->wait_for_service(std::chrono::seconds(1))) {
        if (!rclcpp::ok()) {
            RCLCPP_ERROR(this->get_logger(), "Interrupted while waiting for service.");
            return;
        }
        RCLCPP_INFO(this->get_logger(), "Service not available, waiting again...");
    }
    
    RCLCPP_INFO(this->get_logger(), "Chat Client Node initialized");
}

Chat_Node::~Chat_Node(){
    RCLCPP_INFO(this->get_logger(),"Chat_Node over!\n");
}

/**************************************************************************
函数功能：播放思考提示音(随机选择一个预制音频)
**************************************************************************/
void Chat_Node::playThinkingAudio() {
    try {
        // 获取包路径
        std::string package_path = ament_index_cpp::get_package_share_directory("wheeltec_mic_aiui");
        std::string audio_path = "/home/guest/offline_chat/src/wheeltec_mic/wheeltec_mic_aiui/AIUI/audio/";
        
        // 随机选择一个思考音频(1-3)
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(1, 3);
        int audio_num = dis(gen);
        
        std::string audio_file = audio_path + "ineedtime" + std::to_string(audio_num) + ".wav";

        // 在后台线程中播放音频,避免阻塞
        std::thread([audio_file, this]() {
            std::string play_cmd = "aplay -D plughw:CARD=Device,DEV=0 " + audio_file;
            int result = system(play_cmd.c_str());
            if (result != 0) {
                RCLCPP_WARN(this->get_logger(), "思考提示音播放失败(设备可能被占用)，但不影响对话功能");
            }
        }).detach();
        
        RCLCPP_INFO(this->get_logger(), "正在思考中...");
        
    } catch (const std::exception& e) {
        RCLCPP_WARN(this->get_logger(), "无法播放思考提示音: %s", e.what());
    }
}

int main(int argc, char** argv)
{
    rclcpp::init(argc,argv);
    auto node = std::make_shared<Chat_Node>("chat_node",rclcpp::NodeOptions());
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}
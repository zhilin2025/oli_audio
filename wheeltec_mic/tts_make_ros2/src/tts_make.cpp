#include "wav_head.h"
#include "tts_make.h"

using namespace std;

wstring TTS::s2ws(const std::string &str)
{
	using convert_typeX = std::codecvt_utf8<wchar_t>;
	std::wstring_convert<convert_typeX, wchar_t> converterX;

	return converterX.from_bytes(str);
}

string TTS::ws2s(const std::wstring &wstr)
{
	using convert_typeX = std::codecvt_utf8<wchar_t>;
	std::wstring_convert<convert_typeX, wchar_t> converterX;

	return converterX.to_bytes(wstr);
}

/* 获取时间 */
string TTS::current_time()
{
	std::string fmt = ".wav";
    static char t_buf[64];
	time_t now_time = time(NULL);
    struct tm* time = localtime(&now_time);
    strftime(t_buf, 64, "%Y-%m-%d_%H:%M:%S", time);
    std::wstring wtxt = s2ws(t_buf);
	std::string txt_uft8 = ws2s(wtxt);
    txt_uft8 += fmt;
    return txt_uft8;
}

/* 清理文本中的特殊符号，使其适合TTS播报 */
string TTS::clean_text_for_tts(const string& text)
{
	string cleaned = text;
	
	// 移除 Markdown 加粗/斜体标记 (**)
	size_t pos = 0;
	while ((pos = cleaned.find("**", pos)) != string::npos) {
		cleaned.erase(pos, 2);
	}
	pos = 0;
	while ((pos = cleaned.find("*", pos)) != string::npos) {
		cleaned.erase(pos, 1);
	}
	
	// 移除井号（Markdown标题）
	pos = 0;
	while ((pos = cleaned.find("#", pos)) != string::npos) {
		cleaned.erase(pos, 1);
	}
	
	// 移除竖线分隔符两边的空格并替换为逗号
	pos = 0;
	while ((pos = cleaned.find("|", pos)) != string::npos) {
		cleaned.replace(pos, 1, "，");
	}
	
	// 移除圆括号内的内容（通常是注释）
	pos = 0;
	while ((pos = cleaned.find("（", pos)) != string::npos) {
		size_t end_pos = cleaned.find("）", pos);
		if (end_pos != string::npos) {
			cleaned.erase(pos, end_pos - pos + 3); // UTF-8中文括号是3字节
		} else {
			break;
		}
	}
	
	// 移除英文圆括号内的内容
	pos = 0;
	while ((pos = cleaned.find("(", pos)) != string::npos) {
		size_t end_pos = cleaned.find(")", pos);
		if (end_pos != string::npos) {
			cleaned.erase(pos, end_pos - pos + 1);
		} else {
			break;
		}
	}
	
	// 移除方括号
	pos = 0;
	while ((pos = cleaned.find("[", pos)) != string::npos) {
		cleaned.erase(pos, 1);
	}
	pos = 0;
	while ((pos = cleaned.find("]", pos)) != string::npos) {
		cleaned.erase(pos, 1);
	}
	
	// 将多个连续空格替换为单个空格
	pos = 0;
	while ((pos = cleaned.find("  ", pos)) != string::npos) {
		cleaned.replace(pos, 2, " ");
	}
	
	// 移除开头和结尾的空格
	size_t start = cleaned.find_first_not_of(" \t\n\r");
	size_t end = cleaned.find_last_not_of(" \t\n\r");
	if (start != string::npos && end != string::npos) {
		cleaned = cleaned.substr(start, end - start + 1);
	}
	
	return cleaned;
}

/* 文本合成 */
int TTS::text_to_speech(const char* src_text, const char* des_path, const char* params)
{
	int          ret          = -1;
	FILE*        fp           = NULL;
	const char*  sessionID    = NULL;
	unsigned int audio_len    = 0;
	wave_pcm_hdr wav_hdr      = default_wav_hdr;
	int          synth_status = MSP_TTS_FLAG_STILL_HAVE_DATA;

	if (NULL == src_text || NULL == des_path)
	{
		printf("params is error!\n");
		return ret;
	}
	fp = fopen(des_path, "wb");
	if (NULL == fp)
	{
		printf("open %s error.\n", des_path);
		return ret;
	}
	/* 开始合成 */
	sessionID = QTTSSessionBegin(params, &ret);
	if (MSP_SUCCESS != ret)
	{
		printf("QTTSSessionBegin failed, error code: %d.\n", ret);
		fclose(fp);
		return ret;
	}
	ret = QTTSTextPut(sessionID, src_text, (unsigned int)strlen(src_text), NULL);
	if (MSP_SUCCESS != ret)
	{
		printf("QTTSTextPut failed, error code: %d.\n",ret);
		QTTSSessionEnd(sessionID, "TextPutError");
		fclose(fp);
		return ret;
	}
	printf("正在合成 ...\n");
	fwrite(&wav_hdr, sizeof(wav_hdr) ,1, fp); //添加wav音频头，使用采样率为16000
	while (1) 
	{
		/* 获取合成音频 */
		const void* data = QTTSAudioGet(sessionID, &audio_len, &synth_status, &ret);
		if (MSP_SUCCESS != ret)
			break;
		if (NULL != data)
		{
			fwrite(data, audio_len, 1, fp);
		    wav_hdr.data_size += audio_len; //计算data_size大小
		}
		if (MSP_TTS_FLAG_DATA_END == synth_status)
			break;
	}
	printf("\n");
	if (MSP_SUCCESS != ret)
	{
		printf("QTTSAudioGet failed, error code: %d.\n",ret);
		QTTSSessionEnd(sessionID, "AudioGetError");
		fclose(fp);
		return ret;
	}
	/* 修正wav文件头数据的大小 */
	wav_hdr.size_8 += wav_hdr.data_size + (sizeof(wav_hdr) - 8);
	
	/* 将修正过的数据写回文件头部,音频文件为wav格式 */
	fseek(fp, 4, 0);
	fwrite(&wav_hdr.size_8,sizeof(wav_hdr.size_8), 1, fp); //写入size_8的值
	fseek(fp, 40, 0); //将文件指针偏移到存储data_size值的位置
	fwrite(&wav_hdr.data_size,sizeof(wav_hdr.data_size), 1, fp); //写入data_size的值
	fclose(fp);
	fp = NULL;
	/* 合成完毕 */
	ret = QTTSSessionEnd(sessionID, "Normal");
	if (MSP_SUCCESS != ret)
	{
		printf("QTTSSessionEnd failed, error code: %d.\n",ret);
	}

	return ret;
}

/* 语音合成初始化（只登录MSP，不合成） */
int TTS::init()
{
	int         ret                  = MSP_SUCCESS;
	std::string login_ori		 	 = "appid = ";
	std::string login_fin     	 	 = login_ori + appid + ", work_dir = .";
	const char* login_params = login_fin.c_str();

	// 设置音频输出路径（修改为实际的输出目录）
	std::string audio_path = "src/wheeltec_mic/tts_make_ros2/audio/";
	std::string repalce = "install/tts/share/tts";
	size_t start_pos = source_path.find(repalce);
	if(start_pos != std::string::npos) {
        audio_output_path = source_path;
        audio_output_path.replace(start_pos, repalce.length(), audio_path);
    } else {
		audio_output_path = source_path; // 如果没找到，使用原路径
	}

	/* 用户登录 */
	ret = MSPLogin(NULL, NULL, login_params);
	if (MSP_SUCCESS != ret){
		RCLCPP_ERROR(this->get_logger(), "MSPLogin failed, error code: %d.", ret);
		msp_logged_in = false;
		return ret;
	}

	msp_logged_in = true;
	RCLCPP_INFO(this->get_logger(), "TTS MSP Login Success!");
	RCLCPP_INFO(this->get_logger(), "Resource path: %s", source_path.c_str());
	RCLCPP_INFO(this->get_logger(), "Audio output path: %s", audio_output_path.c_str());
	RCLCPP_INFO(this->get_logger(), "Waiting for feedback_words messages...");

	return 0;
}

/* 合成指定文本 */
int TTS::synthesize_text(const string& text)
{
	if (!msp_logged_in) {
		RCLCPP_ERROR(this->get_logger(), "MSP not logged in!");
		return -1;
	}

	// 清理文本中的特殊符号
	string cleaned_text = clean_text_for_tts(text);
	
	if (cleaned_text.empty()) {
		RCLCPP_WARN(this->get_logger(), "清理后的文本为空，跳过合成");
		return 0;
	}

	// 构建 session 参数字符串（使用原始的 source_path 作为资源路径）
	std::string session_params = "engine_type = local,voice_name=" + voice_name + 
		", text_encoding = UTF8, tts_res_path = fo|" + source_path + 
		"/config/bin/msc/res/tts/xiaoyan.jet;fo|" + source_path + 
		"/config/bin/msc/res/tts/common.jet, sample_rate = " + std::to_string(sample_rate) + 
		", volume = " + std::to_string(volume) + 
		", pitch = " + std::to_string(pitch) + 
		", rdn = " + std::to_string(rdn) + 
		", speed = " + std::to_string(speed);

	// 构建输出文件名（使用 audio_output_path）
	std::string filename = audio_output_path + current_time();

	RCLCPP_INFO(this->get_logger(), "原始文本: %s", text.c_str());
	RCLCPP_INFO(this->get_logger(), "清理后文本: %s", cleaned_text.c_str());
	RCLCPP_INFO(this->get_logger(), "输出文件: %s", filename.c_str());
	
	int ret = text_to_speech(cleaned_text.c_str(), filename.c_str(), session_params.c_str());
	if (MSP_SUCCESS != ret)
	{
		RCLCPP_ERROR(this->get_logger(), "text_to_speech failed, error code: %d.", ret);
		return ret;
	}
	RCLCPP_INFO(this->get_logger(), "合成完毕");

	// 播放音频文件（同步播放并在播放完成后删除）
	// std::string play_cmd = "aplay " + filename + " && rm -f " + filename;
	std::string play_cmd = "aplay -D plughw:Device,0 " + filename + " && sleep 0.5 && rm -f " + filename;
	RCLCPP_INFO(this->get_logger(), "正在播放音频...");
	
	// 使用后台进程播放并清理
	std::thread([play_cmd, this]() {
		int result = system(play_cmd.c_str());
		if (result == 0) {
			RCLCPP_INFO(this->get_logger(), "音频播放完成并已清理");
		} else {
			RCLCPP_WARN(this->get_logger(), "音频播放或清理失败");
		}
	}).detach();

	return 0;
}

/* 话题回调函数 */
void TTS::feedback_words_callback(const std_msgs::msg::String::SharedPtr msg)
{
	if (msg->data.empty()) {
		return;
	}
	RCLCPP_INFO(this->get_logger(), "收到反馈文本: %s", msg->data.c_str());
	synthesize_text(msg->data);
}

/* 初始化 */
TTS::TTS(const std::string &node_name,const rclcpp::NodeOptions &options) 
: rclcpp::Node(node_name,options), msp_logged_in(false){
	RCLCPP_INFO(this->get_logger(),"%s node init!\n",node_name.c_str());

	this->declare_parameter<int>("rdn",0);
	this->declare_parameter<int>("volume",0);
	this->declare_parameter<int>("pitch",0);
	this->declare_parameter<int>("speed",0);
	this->declare_parameter<int>("sample_rate",0);
	this->declare_parameter<string>("source_path","");
	this->declare_parameter<string>("appid","");
	this->declare_parameter<string>("voice_name","");
	this->declare_parameter<string>("tts_text","");

	this->get_parameter("rdn",rdn);
	this->get_parameter("volume",volume);
	this->get_parameter("pitch",pitch);
	this->get_parameter("speed",speed);
	this->get_parameter("sample_rate",sample_rate);
	this->get_parameter<string>("source_path",source_path);
	this->get_parameter<string>("appid",appid);
	this->get_parameter<string>("voice_name",voice_name);
	this->get_parameter<string>("tts_text",tts_text);

	// 创建订阅者
	feedback_words_sub = this->create_subscription<std_msgs::msg::String>(
		"feedback_words", 10, 
		std::bind(&TTS::feedback_words_callback, this, std::placeholders::_1));
}

TTS::~TTS(){
	if (msp_logged_in) {
		MSPLogout();
	}
	RCLCPP_INFO(this->get_logger(),"tts_node over!\n");
} 

int main(int argc,char **argv)
{
	rclcpp::init(argc,argv);
	auto tts_node = std::make_shared<TTS>("tts_node", rclcpp::NodeOptions());
	
	if (tts_node->init() == 0){
		RCLCPP_INFO(tts_node->get_logger(), "TTS node initialized, spinning...");
		rclcpp::spin(tts_node);
	}
	else{
		RCLCPP_ERROR(tts_node->get_logger(), "TTS node initialization failed!");
	}
	
	rclcpp::shutdown();
	return 0;
}

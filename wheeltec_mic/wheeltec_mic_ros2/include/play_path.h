#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#include <string>

// std::string head = "pasuspender -- aplay -q -D plughw:2,0 ";
// std::string head = "aplay ";
// std::string head = "aplay -q -D plughw:2,0 ";
// 通过aplay -l看看用哪个设备来播放声音，直接使用设备卡名，不受卡号的影响，解决每次开关机卡号重分配的问题
std::string head = "aplay -D plughw:Device,0 ";
std::string audio_path;
std::string WHOLE;

#endif
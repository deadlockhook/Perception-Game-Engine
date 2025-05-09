#include "s_string.h"   


sw_string s_string::to_swstring() const {
int len = MultiByteToWideChar(CP_UTF8, 0, c_str(), (int)m_length, nullptr, 0);
sw_string wstr;
wstr.reserve(len + 1);
MultiByteToWideChar(CP_UTF8, 0, c_str(), (int)m_length, (wchar_t*)wstr.begin(), len);
wstr.push_back(L'\0');
wstr.pop_back();
return wstr;
   }

s_string sw_string::to_sstring() const {
    int len = WideCharToMultiByte(CP_UTF8, 0, c_str(), (int)m_length, nullptr, 0, nullptr, nullptr);
    s_string str;
    str.reserve(len + 1);
    WideCharToMultiByte(CP_UTF8, 0, c_str(), (int)m_length, (char*)str.begin(), len, nullptr, nullptr);
    str.push_back('\0');
    str.pop_back(); 
    return str;
}
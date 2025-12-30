#include "idiom_dict.h"
#include <fstream>
#include <algorithm>
#include <cctype>

// 去除首尾空白
static inline std::string trim(const std::string &s) {
    size_t a = 0, b = s.size();
    while (a < b && std::isspace((unsigned char)s[a])) ++a; // 去除前导空白
    while (b > a && std::isspace((unsigned char)s[b-1])) --b; // 去除尾随空白
    return s.substr(a, b-a);
}

// 从 UTF-8 文件加载四字成语到内存结构中：
// by_first_ : 以首字为键的候选成语列表；all_ : 全集用于快速判断
bool IdiomDict::load(const std::string &path) {
    std::ifstream fin(path); // 以文本模式打开
    if (!fin) return false;
    std::string line; // 逐行读取
    while (std::getline(fin, line)) {
        std::string s = trim(line); // 去除首尾空白
        if (s.empty()) continue; // 跳过空行
        if (utf8_len(s) != 4) continue;// 仅接受严格的四个 UTF-8 字符（四字成语）
        std::string f = first_char(s); // 首字
        by_first_[f].push_back(s); // 加入以首字为键的候选列表
        all_.insert(s); // 加入全集
    }
    return !by_first_.empty(); // 至少加载到一个四字成语则视为成功
}

// 计算 UTF-8 字符数（以 codepoint 计数）
int IdiomDict::utf8_len(const std::string &s) const {
    int cnt = 0;
    for (unsigned char c : s) if ((c & 0xC0) != 0x80) ++cnt; // 非续字节计数
    return cnt;
}

// 获取 UTF-8 字符串的首字
std::string IdiomDict::first_char(const std::string &s) const {
    if (s.empty()) return std::string(); // 空字符串返回空
    unsigned char c = s[0]; // 读取首字节
    size_t len = 1; // 计算首字长度
    if ((c & 0x80) == 0) len = 1; // ASCII 字符 （单字节 UTF-8）
    else if ((c & 0xE0) == 0xC0) len = 2; // 2 字节 UTF-8（拉丁文等）
    else if ((c & 0xF0) == 0xE0) len = 3; // 3 字节 UTF-8（汉字常见）
    else if ((c & 0xF8) == 0xF0) len = 4; // 4 字节 UTF-8 （少见）
    return s.substr(0, len); // 返回首字子串
}

// 获取 UTF-8 字符串的尾字
std::string IdiomDict::last_char(const std::string &s) const {
    if (s.empty()) return std::string(); // 空字符串返回空
    size_t i = s.size(); // 从字符串末尾向前扫描至首字节
    while (i > 0) {
        unsigned char c = s[i-1]; // 读取当前字节
        if ((c & 0xC0) != 0x80) break;  // 非续字节即为首字节，停止扫描
        --i; // 否则继续向前扫描
    }
    unsigned char lead = s[i-1]; // 读取尾字首字节
    size_t len = 1; // 计算尾字长度
    if ((lead & 0x80) == 0) len = 1; // 同理
    else if ((lead & 0xE0) == 0xC0) len = 2;
    else if ((lead & 0xF0) == 0xE0) len = 3;
    else if ((lead & 0xF8) == 0xF0) len = 4;
    return s.substr(s.size()-len, len);
}

// 判断字符串是否为成语
bool IdiomDict::is_idiom(const std::string &s) const {
    return all_.count(s) != 0; // 在全集中查找
}

// 获取以 first 开头的成语列表
const std::vector<std::string> &IdiomDict::candidates_for(const std::string &first) const {
    static const std::vector<std::string> empty; // 若无候选则返回空列表
    auto it = by_first_.find(first); // 查找首字对应的候选列表
    if (it == by_first_.end()) return empty; // 没找到，it == by_first_.end() 为真
    return it->second; // 返回候选列表引用，second 为值部分
}

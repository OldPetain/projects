#ifndef IDIOM_DICT_H
#define IDIOM_DICT_H

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

// 成语词库类：支持从 UTF-8 编码文件加载成语，
class IdiomDict {
public:
    bool load(const std::string &path); // 从文件加载成语
    bool is_idiom(const std::string &s) const; // 判断是否为成语
    int utf8_len(const std::string &s) const; // 计算 UTF-8 字符数
    std::string first_char(const std::string &s) const; // 获取首字
    std::string last_char(const std::string &s) const; // 获取尾字
    const std::vector<std::string> &candidates_for(const std::string &first) const; // 获取以 first 开头的成语列表

private:
    std::unordered_map<std::string, std::vector<std::string>> by_first_; // 以首字为键的候选成语列表
    std::unordered_set<std::string> all_; // 全集用于快速判断
};

#endif // IDIOM_DICT_H

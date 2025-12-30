#ifndef GAME_H
#define GAME_H

#include "idiom_dict.h"
#include <unordered_set>

// 成语接龙游戏类：管理游戏状态与主循环
class Game {
public:
    explicit Game(IdiomDict &dict); // 构造函数，传入成语词库引用
    void run(); // 运行游戏主循环

private:
    IdiomDict &dict_; // 成语词库引用
    std::unordered_set<std::string> used_; // 已使用成语集合
    void prompt(const std::string &msg) const; // 输出提示信息
    // 显示以 first 字开头的至多 max 个可接成语示例
    void show_help_for(const std::string &first, int max_examples = 3) const;
    // 从玩家读取并验证输入，要求首字为 expected_first。
    // 返回 true 表示读取到合法成语并写入 out；返回 false 表示遇到退出/EOF或非法输入（游戏结束）。
    bool read_player_input(const std::string &expected_first, std::string &out);
};

#endif // GAME_H

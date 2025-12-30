#include "game.h"
#include <iostream>
#include <string>
#include <random>
#include <algorithm>

// 构造函数：保存成语词库引用
Game::Game(IdiomDict &dict) : dict_(dict) {}

// 简单的输出提示函数
void Game::prompt(const std::string &msg) const {
    std::cout << msg; // 输出提示
    std::cout.flush(); // 刷新输出缓冲区
}

// 显示以 first 字开头的至多 max_examples 个可接成语示例（排除已用）
void Game::show_help_for(const std::string &first, int max_examples) const {
    const auto &cands = dict_.candidates_for(first);
    std::vector<std::string> avail;
    for (const auto &c : cands) if (!used_.count(c)) avail.push_back(c);
    if (avail.empty()) {
        std::cout << "没有可接成语\n";
        return;
    }
    std::cout << "示例：";
    for (size_t i = 0; i < avail.size() && (int)i < max_examples; ++i) {
        if (i) std::cout << "，";
        std::cout << avail[i];
    }
    std::cout << "\n";
}

// 读取玩家输入并验证（允许输入 help），返回 true 则 out 为合法成语；返回 false 表示退出或非法导致游戏结束
bool Game::read_player_input(const std::string &expected_first, std::string &out) {
    std::string line; // 读取玩家输入行
    while (true) {
        if (!std::getline(std::cin, line)) return false; // EOF
        // 处理退出命令
        if (line == "q" || line == "quit") return false;
        // 去除首尾空白
        size_t a = line.find_first_not_of(" \t\r\n");
        if (a==std::string::npos) { std::cout << "玩家输入的不是成语\n"; return false; }
        size_t b = line.find_last_not_of(" \t\r\n");
        std::string raw = line.substr(a, b-a+1);
        // help 命令（不区分大小写）
        std::string lower = raw;
        std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c){ return std::tolower(c); }); // 转小写
        if (lower == "help") {
            show_help_for(expected_first);
            std::cout << "请重新以 '" << expected_first << "' 开头接成语：";
            continue; // 继续提示玩家输入
        }
        // 校验格式（四字）和是否在词库中
        if (dict_.utf8_len(raw) != 4) { std::cout << "玩家输入的不是成语\n"; return false; }
        if (!dict_.is_idiom(raw)) { std::cout << "玩家输入的不是成语\n"; return false; }
        // 校验首字是否符合要求
        if (dict_.first_char(raw) != expected_first) { std::cout << "玩家无法接龙\n"; return false; }
        out = raw;
        return true;
    }
}

// 主循环：
void Game::run() {
    std::cout << "成语接龙开始。输入四字成语（输入 q 或 quit 退出）。\n";
    std::string line; // 读取玩家输入行
    std::random_device rd; // 随机数种子
    std::mt19937 rng(rd()); // Mersenne Twister 伪随机数生成器

    // 首次由玩家输入一个成语作为起始（避免每轮重复提示）
    prompt("玩家请先输入一个成语：");
    if (!std::getline(std::cin, line)) { std::cout << "游戏结束。\n"; return; } // 读取输入
    // 去除首尾空白
    size_t a = line.find_first_not_of(" \t\r\n");
    if (a==std::string::npos) { std::cout << "玩家输入的不是成语\n"; return; }
    size_t b = line.find_last_not_of(" \t\r\n");
    line = line.substr(a, b-a+1);
    // 确认是成语再加入已使用集合
    if (line == "q" || line == "quit") { std::cout << "游戏结束。\n"; return; }
    if (dict_.utf8_len(line) != 4) { std::cout << "玩家输入的不是成语\n"; return; }
    if (!dict_.is_idiom(line)) { std::cout << "玩家输入的不是成语\n"; return; }
    used_.insert(line);

    // 进入交替循环：程序回复 -> 玩家接 -> 程序回复 -> ...
    while (true) {
        // 程序回复
        std::string last = dict_.last_char(line); // 取玩家输入成语的尾字
        const auto &cands = dict_.candidates_for(last); // 获取以该字开头的候选成语
        std::vector<std::string> avail; // 可用成语列表
        for (const auto &c : cands) if (!used_.count(c)) avail.push_back(c); // 将未使用的候选成语加入可用列表
        if (avail.empty()) { std::cout << "程序无法接龙\n"; break; }
        std::shuffle(avail.begin(), avail.end(), rng); // 随机打乱可用成语顺序
        std::string reply = avail.front(); // 选择第一个成语作为回复
        std::cout << "程序接：" << reply << "\n";
        used_.insert(reply); // 标记该成语为已使用

        // 等待玩家以 reply 的尾字开头接成语，支持 help 命令
        std::string player_input;
        std::string expect_first = dict_.last_char(reply);
        prompt(std::string("请以 '") + expect_first + "' 开头接成语：");
        if (!read_player_input(expect_first, player_input)) break; // 读取失败或非法，结束游戏
        used_.insert(player_input);
        line = player_input; // 玩家有效输入，继续下一轮
    }
    std::cout << "游戏结束。\n";
}

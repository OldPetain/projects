// 主程序：优先使用用户传入路径，否则尝试使用当前目录下的 `成语词库t.txt`。
// 若词库为 GBK 编码，程序在运行时会尝试用系统上的 `iconv` 转换为 UTF-8 后再加载。
#include <iostream>
#include <fstream>
#include <string>
#include <unistd.h>
#include <stdlib.h>
#include "idiom_dict.h"
#include "game.h"

// 将文件通过 iconv 转换为 UTF-8，输出到临时文件并返回临时文件路径；失败返回空字符串
static std::string convert_to_utf8_tmp(const std::string &path) {
    char tmp[] = "/tmp/idiom_utf8_XXXXXX"; // 临时文件模板
    int fd = mkstemp(tmp); // 创建临时文件
    if (fd == -1) return std::string(); // 创建失败
    close(fd); // 关闭文件描述符，iconv 会重新打开文件
    std::string tmpname(tmp); // 临时文件名字符串
    std::string cmd = "iconv -f gbk -t utf-8 '" + path + "' -o '" + tmpname + "' 2>/dev/null"; // 转换命令
    int rc = system(cmd.c_str()); // 执行命令
    if (rc != 0) {  // 转换失败
        unlink(tmpname.c_str()); // 删除临时文件
        return std::string(); // 返回空字符串
    }
    return tmpname; // 返回临时文件名
}

int main(int argc, char **argv) {
    std::string dict_path = (argc >= 2) ? argv[1] : std::string("成语词库t.txt"); // 成语词库路径

    IdiomDict dict; // 创建成语词库实例
    // 先尝试直接以 UTF-8 加载（若词库已是 UTF-8，则会成功）
    if (!dict.load(dict_path)) {
        // 若失败，则尝试用 iconv 将 GBK 转为 UTF-8 并加载临时文件
        std::string tmp = convert_to_utf8_tmp(dict_path); 
        if (tmp.empty() || !dict.load(tmp)) {
            std::cerr << "无法打开或加载成语库（尝试转换也失败）：" << dict_path << "\n";
            return 1;
        }
        // 成功加载后删除临时文件
        unlink(tmp.c_str());
    }

    Game game(dict); // 创建游戏实例
    game.run(); // 运行游戏主循环
    return 0;
}

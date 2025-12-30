#pragma once
#include <vector>
#include <string>

// 结果结构体, 包含总路程和欧拉回路顶点序列
struct CPPResult {
    double total_cost; // 总共路程长度
    std::vector<int> euler_path; // 欧拉回路顶点序列
};

// 中国邮路问题类定义
class ChinesePostman {
public:
    // 下载图数据从文件; 期望格式: n m, 随后 m 行: u v w (1-based)
    bool loadFromFile(const std::string &path);
    // 从标准输入加载图数据; 期望格式同上
    bool loadFromStdin();

    // 使用给定起点(0-based)求解中国邮路问题。返回CPPResult。如果无法求解，total_cost < 0。
    CPPResult solve(int start = 0);

private:
    int n = 0; // 顶点数
    struct Edge { int u,v; double w; };// 边结构体：起点u，终点v，权重w
    std::vector<Edge> edges; // 原始边，以向量形式存储更多
    // 内部辅助函数
    bool isConnected(int start) const;// 判断图在度>0 的顶点间是否连通
};

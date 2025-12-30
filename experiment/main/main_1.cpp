/*
#include <iostream> // 用于输入输出
#include "poly.h" 

int main() {
    Polynomial poly;
    std::cout << "请输入系数 与 指数 的成对数值（以 0 0 结束或用 EOF）：\n";

    double coeff;
    int exp;
    while (std::cin >> coeff >> exp) {
        if (coeff == 0.0 && exp == 0) break; // 允许用 0 0 作为结束标志
        poly.insertTerm(coeff, exp);
    }

    Polynomial der = poly.derivative();

    std::cout << "求导前的多项式为：" << poly.toString() << std::endl;
    std::cout << "求导后的多项式为：" << der.toString() << std::endl;
    return 0;
}
*/

#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <limits>
#include "poly.h"

// 从标准输入读取多项式（成对 coeff exp，以 0 0 结束）
static void readPolynomial(Polynomial &p) {
    std::cout << "输入系数 指数 成对（以 0 0 结束）：\n";
    double c; int e;
    while (true) {
        if (!(std::cin >> c >> e)) { // 输入错误处理
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');  // 清除错误状态和剩余输入
            std::cout << "输入格式错误，请重新输入一对数值（coeff exp），或输入 0 0 结束。\n";
            continue;
        }
        if (c == 0.0 && e == 0) break;
        p.insertTerm(c, e);
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

// 保存多项式的文本表示到文件（每行一个多项式的 toString 输出）
static void saveAllToFile(const std::vector<Polynomial> &polys, const std::string &path = "polynomials.txt") {
    std::ofstream ofs(path, std::ios::trunc);
    if (!ofs) {
        std::cerr << "无法打开文件 " << path << " 进行写入。\n";
        return;
    }
    for (size_t i = 0; i < polys.size(); ++i) {
        ofs << (i + 1) << ": " << polys[i].toString() << "\n";
    }
    std::cout << "已导出 " << polys.size() << " 个多项式到 " << path << "\n";
}

// 打印已保存的多项式列表
static void listPolynomials(const std::vector<Polynomial> &polys) {
    if (polys.empty()) {
        std::cout << "当前没有已保存的多项式。\n";
        return;
    }
    std::cout << "已保存多项式列表：\n";
    for (size_t i = 0; i < polys.size(); ++i) {
        std::cout << (i + 1) << ": " << polys[i].toString() << "\n";
    }
}

// 安全读取一个编号，范围 [1, maxIndex]
static bool readIndex(const std::string &prompt, size_t maxIndex, size_t &outIdx) {
    if (maxIndex == 0) return false;
    while (true) {
        std::cout << prompt;
        long long v;
        if (!(std::cin >> v)) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "输入无效，请输入数字。\n";
            continue;
        }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        if (v < 1 || (size_t)v > maxIndex) {
            std::cout << "编号超出范围（1-" << maxIndex << "），请重试。\n";
            continue;
        }
        outIdx = (size_t)v;
        return true;
    }
}

int main() {
    std::vector<Polynomial> polys;
    for (;;) {
        std::cout << "\n请选择功能：\n"
                  << "1. 创建新多项式并保存到内存\n"
                  << "2. 查看已有多项式\n"
                  << "3. 对两个多项式做乘法并保存结果到内存\n"
                  << "4. 对已保存多项式求导并保存结果到内存\n"
                  << "5. 删除某个已保存的多项式\n"
                  << "6. 导出到文件 polynomials.txt（手动保存）\n"
                  << "7. 退出\n"
                  << "输入选项编号：";
        int opt;
        if (!(std::cin >> opt)) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "读取选项失败，请重试。\n";
            continue;
        }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        if (opt == 1) {
            Polynomial p;
            readPolynomial(p);
            polys.push_back(std::move(p));
            std::cout << "已保存编号 " << polys.size() << " : " << polys.back().toString() << "\n";
        } else if (opt == 2) {
            listPolynomials(polys);
        } else if (opt == 3) {
            if (polys.size() < 2) {
                std::cout << "至少需要两个已保存的多项式（请先创建）。\n";
                continue;
            }
            listPolynomials(polys);
            size_t a, b;
            if (!readIndex("输入第一个多项式编号：", polys.size(), a)) continue;
            if (!readIndex("输入第二个多项式编号：", polys.size(), b)) continue;
            std::cout << "选择乘法实现：1. Naive  2. Hash （默认 2）：";
            int method = 2;
            if (!(std::cin >> method)) {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                method = 2;
            } else {
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            }
            Polynomial res;
            if (method == 1) res = polys[a-1].multiplyNaive(polys[b-1]);
            else res = polys[a-1].multiplyHash(polys[b-1]);
            polys.push_back(std::move(res));
            std::cout << "乘积已保存为编号 " << polys.size() << " : " << polys.back().toString() << "\n";
        } else if (opt == 4) {
            if (polys.empty()) {
                std::cout << "当前没有已保存的多项式。\n";
                continue;
            }
            listPolynomials(polys);
            size_t idx;
            if (!readIndex("请输入要求导的多项式编号：", polys.size(), idx)) continue;
            Polynomial der = polys[idx-1].derivative();
            polys.push_back(std::move(der));
            std::cout << "导数已保存为编号 " << polys.size() << " : " << polys.back().toString() << "\n";
        } else if (opt == 5) {
            if (polys.empty()) {
                std::cout << "当前没有已保存的多项式可删除。\n";
                continue;
            }
            listPolynomials(polys);
            size_t idx;
            if (!readIndex("请输入要删除的多项式编号：", polys.size(), idx)) continue;
            std::cout << "确认删除编号 " << idx << " : " << polys[idx-1].toString() << " ? (y/n): ";
            char ch;
            if (!(std::cin >> ch)) { std::cin.clear(); std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); continue; }
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            if (ch == 'y' || ch == 'Y') {
                polys.erase(polys.begin() + (idx - 1));
                std::cout << "已删除。\n";
            } else {
                std::cout << "已取消删除。\n";
            }
        } else if (opt == 6) {
            saveAllToFile(polys);
        } else if (opt == 7) {
            std::cout << "退出。\n";
            break;
        } else {
            std::cout << "无效选项。\n";
        }
    }
    return 0;
}
#include <iostream>
#include <string>
#include <cmath>
#include <iomanip>
#include <limits>
#include "../include/expr.h"

// 交互菜单：用户可以输入先序（支持带空格的 token，也兼容单字符格式），查看中缀/后缀，计算值，释放树等。
int main() {
    ExprNode* root = nullptr;
    for (;;) {
        std::cout << "\n表达式求值工具 - 请选择操作：\n"
              << "1. 输入先序(带 '#' 标记空子树；支持空格分隔 token，用于多位数)\n"
              << "2. 显示中缀表达式（带括号）\n"
              << "3. 显示后缀表达式（空格分隔 token）\n"
              << "4. 计算表达式树值（中缀所表示的值）\n"
              << "5. 计算后缀表达式值\n"
              << "6. 释放当前表达式树\n"
              << "7. 退出\n"
              << "8. 直接从中缀转换为后缀（不构建树）\n"
              << "9. 直接对中缀表达式求值（不构建树）\n"
              << "输入选项编号：";

        int opt = 0;
        if (!(std::cin >> opt)) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "输入无效，请输入数字 (1-7)。\n";
            continue;
        }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        switch (opt) {
            case 1: {
                std::cout << "请输入先序序列 (例如：+/*+1##2##-5##2##2##%5##3# 或使用空格分隔 token：+ * ( 1 2 # # ) ...)：\n";
                std::string line;
                if (!std::getline(std::cin, line)) { std::cout << "读取失败。\n"; break; }
                if (line.empty()) { std::cout << "输入为空，取消。\n"; break; }
                if (root) { freeExprTree(root); root = nullptr; }
                root = buildExprTreeFromPreorder(line);
                if (!root) std::cout << "构建表达式树失败（可能输入格式不正确）。\n";
                else std::cout << "表达式树已构建。\n";
                break;
            }
            case 2: {
                if (!root) { std::cout << "当前没有表达式树，请先选择 1 输入。\n"; break; }
                try {
                    std::string infix = exprToInfix(root);
                    std::cout << "中缀表达式：" << infix << "\n";
                } catch (const std::exception &e) {
                    std::cout << "生成中缀表达式时出错：" << e.what() << "\n";
                }
                break;
            }
            case 3: {
                if (!root) { std::cout << "当前没有表达式树，请先选择 1 输入。\n"; break; }
                try {
                    std::string postfix = exprToPostfix(root);
                    std::cout << "后缀表达式：" << postfix << "\n";
                } catch (const std::exception &e) {
                    std::cout << "生成后缀表达式时出错：" << e.what() << "\n";
                }
                break;
            }
            case 4: {
                if (!root) { std::cout << "当前没有表达式树，请先选择 1 输入。\n"; break; }
                try {
                    double val = evalExprTree(root);
                    std::cout.setf(std::ios::fixed); std::cout<<std::setprecision(2);
                    std::cout << "表达式树求值结果：" << val << "\n";
                } catch (const std::exception &e) {
                    std::cout << "求值出错：" << e.what() << "\n";
                }
                break;
            }
            case 5: {
                if (!root) { std::cout << "当前没有表达式树，请先选择 1 输入。\n"; break; }
                try {
                    std::string postfix = exprToPostfix(root);
                    double val = evalPostfixExpr(postfix);
                    double intpart;
                    if (std::modf(val, &intpart) == 0.0) {
                        std::cout.unsetf(std::ios::floatfield);
                        std::cout << "后缀表达式求值结果：" << (long long)intpart << "\n";
                    } else {
                        std::cout.setf(std::ios::fixed); std::cout<<std::setprecision(2);
                        std::cout << "后缀表达式求值结果：" << val << "\n";
                    }
                } catch (const std::exception &e) {
                    std::cout << "后缀求值出错：" << e.what() << "\n";
                }
                break;
            }
            case 6: {
                if (!root) { std::cout << "当前没有表达式树，无需释放。\n"; break; }
                freeExprTree(root); root = nullptr;
                std::cout << "已释放当前表达式树。\n";
                break;
            }
            case 8: {
                // infix -> postfix (shunting-yard)
                std::cout << "请输入中缀表达式（支持 + - * / % ^ 和括号）:\n";
                std::string line;
                if (!std::getline(std::cin, line)) { std::cout << "读取失败。\n"; break; }
                if (line.empty()) { std::cout << "输入为空，取消。\n"; break; }
                try {
                    std::string postfix = infixToPostfix(line);
                    std::cout << "后缀表达式: " << postfix << "\n";
                } catch (const std::exception &e) {
                    std::cout << "转换失败: " << e.what() << "\n";
                }
                break;
            }
            case 9: {
                // infix evaluation
                std::cout << "请输入中缀表达式（支持 + - * / % ^ 和括号）:\n";
                std::string line;
                if (!std::getline(std::cin, line)) { std::cout << "读取失败。\n"; break; }
                if (line.empty()) { std::cout << "输入为空，取消。\n"; break; }
                try {
                    double val = evalInfixExpr(line);
                    double intpart;
                    if (std::modf(val, &intpart) == 0.0) {
                        std::cout.unsetf(std::ios::floatfield);
                        std::cout << "中缀求值结果: " << (long long)intpart << "\n";
                    } else {
                        std::cout.setf(std::ios::fixed); std::cout<<std::setprecision(6);
                        std::cout << "中缀求值结果: " << val << "\n";
                    }
                } catch (const std::exception &e) {
                    std::cout << "求值失败: " << e.what() << "\n";
                }
                break;
            }
            case 7: {
                if (root) freeExprTree(root);
                std::cout << "退出。\n";
                return 0;
            }
            default: {
                std::cout << "无效选项，请输入 1-7。\n";
                break;
            }
        }
    }
    return 0;
}
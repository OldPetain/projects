#pragma once
#include <string>

// 构建表达式二叉树（先序序列，使用 '#' 标记空子树）
// 返回根指针；需要调用者在不再使用时释放树（使用 freeExprTree）。
struct ExprNode;
ExprNode* buildExprTreeFromPreorder(const std::string &s);

// 释放树内存
void freeExprTree(ExprNode* root);

// 以带括号的中缀形式返回表达式（例如 (1+2)）
std::string exprToInfix(ExprNode* root);

// 返回后缀表达式（不带空格，操作数为单字符）
std::string exprToPostfix(ExprNode* root);

// 递归求值（除法为双精度，取模使用整数模）
double evalExprTree(ExprNode* root);

// 根据后缀表达式求值（操作数为单字符数字）
double evalPostfixExpr(const std::string &postfix);

// 直接将中缀表达式转换为后缀表达式（Shunting-yard 算法）
// 支持运算符：+ - * / % ^ 以及圆括号 ( )；操作数为单字符数字（与当前实现兼容）
std::string infixToPostfix(const std::string &infix);

// 直接对中缀表达式求值（内部先转换为后缀再计算）
double evalInfixExpr(const std::string &infix);

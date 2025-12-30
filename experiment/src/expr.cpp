#include "expr.h"
#include <cctype>
#include <cmath>
#include <stdexcept>
#include <string>
#include <vector>

// 定义表达式树节点
struct ExprNode {
    char val;
    ExprNode* left;
    ExprNode* right;
    ExprNode(char c): val(c), left(nullptr), right(nullptr) {}
};

// 辅助函数：从先序序列构建表达式树
static ExprNode* buildFromPre(const std::string &s, size_t &pos) { //输入：先序字符串和当前位置引用
    while (pos < s.size() && isspace((unsigned char)s[pos])) ++pos; // 跳过空白
    if (pos >= s.size()) return nullptr; // 越界检查
    char c = s[pos++]; // 读取当前字符
    if (c == '#') return nullptr; // 空子树标记
    ExprNode* node = new ExprNode(c); // 创建新节点
    node->left = buildFromPre(s, pos); // 构建左子树（先序是这样的）
    node->right = buildFromPre(s, pos); // 构建右子树
    return node;
}

// 从先序序列构建表达式树
ExprNode* buildExprTreeFromPreorder(const std::string &s) {
    size_t pos = 0; // 初始化位置索引
    return buildFromPre(s, pos); // 从位置0开始构建树
}

void freeExprTree(ExprNode* root) {
    if (!root) return; // 空节点无需释放
    freeExprTree(root->left); // 递归释放左子树
    freeExprTree(root->right); // 递归释放右子树
    delete root; // 释放当前节点
}

// 优化的中缀输出：根据优先级和结合性只添加必要的括号，减少冗余括号
static int prec(char op) { // 获得运算符优先级
    if (op == '+' || op == '-') return 1;
    if (op == '*' || op == '/' || op == '%') return 2;
    if (op == '^') return 3; // 幂运算优先级最高
    return 100; // 叶子或未知
}

static bool isOpChar(char c) {
    return c=='+'||c=='-'||c=='*'||c=='/'||c=='%'||c=='^';
}

// 是否为右结合运算符（幂运算 '^' 是右结合）
static bool isRightAssoc(char c) {
    return c == '^';
}

// 递归生成中缀表达式
static std::string exprToInfixRec(ExprNode* node) {
    if (!node) return std::string(); // 空节点返回空字符串
    if (!node->left && !node->right) return std::string(1, node->val); // 叶节点直接返回值字符

    char op = node->val; // 当前节点运算符
    std::string L = exprToInfixRec(node->left); // 递归获取左子表达式
    std::string R = exprToInfixRec(node->right); // 递归获取右子表达式

    if (node->left && isOpChar(node->left->val)) { // 左子节点是运算符
        int pl = prec(node->left->val);
        // 如果左子树优先级小于当前，需要加括号；
        // 如果优先级相等且父运算符为右结合（如 ^），左子表达式需加括号
        if (pl < prec(op) || (pl == prec(op) && isRightAssoc(op))) L = std::string("(") + L + ")";
    }
    if (node->right && isOpChar(node->right->val)) { // 右子节点是运算符
        int pr = prec(node->right->val);
        if (pr < prec(op)) R = std::string("(") + R + ")"; // 右子优先级小于当前需加括号
        else if (pr == prec(op)) { // 优先级相等时
            // 减法、除法、取模为左结合，右子表达式需加括号，若是右结合则不需
            if (!isRightAssoc(op) && (op == '-' || op == '/' || op == '%')) R = std::string("(") + R + ")"; 
        }
    }
    return L + op + R;
}

std::string exprToInfix(ExprNode* root) {
    return exprToInfixRec(root); // 调用递归函数生成中缀表达式
}

// 辅助函数：后序遍历构建后缀表达式
void postfixRec(ExprNode* node, std::string &out) {
    if (!node) return; // 空节点直接返回
    postfixRec(node->left, out); // 递归遍历左子树
    postfixRec(node->right, out); // 递归遍历右子树
    out.push_back(node->val); // 访问当前节点
}

// 返回后缀表达式
std::string exprToPostfix(ExprNode* root) {
    std::string out; // 初始化输出字符串
    postfixRec(root, out); // 调用辅助函数进行后序遍历
    return out;
}

// -------------------- 中缀转后缀（Shunting-yard） --------------------
// 支持运算符：+ - * / % ^ 以及括号。假设操作数为单字符数字，与 evalPostfixExpr 相兼容。
std::string infixToPostfix(const std::string &infix) {
    std::string out; // 输出后缀表达式
    std::vector<char> ops; // 运算符栈
    auto topOp = [&]() -> char { return ops.empty() ? '\0' : ops.back(); }; // 获取栈顶运算符

    // 辅助函数：判断是否为运算符，获取优先级，判断结合性
    auto isOp = [](char c) { return c=='+'||c=='-'||c=='*'||c=='/'||c=='%'||c=='^'; }; // 判断是否为运算符
    auto precedence = [&](char c)->int { return prec(c); }; // 获取运算符优先级
    auto rightAssoc = [&](char c)->bool { return isRightAssoc(c); }; // 判断是否为右结合运算符

    for (size_t i = 0; i < infix.size(); ++i) { // 遍历中缀表达式
        char c = infix[i]; // 当前字符
        if (isspace((unsigned char)c)) continue; // 跳过空白
        if (isdigit((unsigned char)c)) {
            out.push_back(c);  // 操作数直接加入输出
            continue;
        }
        if (c == '(') { ops.push_back(c); continue; } // 左括号入栈
        if (c == ')') { // 右括号处理
            while (!ops.empty() && topOp() != '(') { //当栈不为空且栈顶不是左括号时
                out.push_back(topOp()); ops.pop_back(); // 弹出运算符加入输出
            }
            if (!ops.empty() && topOp() == '(') ops.pop_back();  // 弹出左括号
            continue;
        }
        if (isOp(c)) { // 当前字符是运算符  
            while (!ops.empty() && isOp(topOp())) {
                char o2 = topOp(); // 栈顶运算符
                int p1 = precedence(c); // 当前运算符优先级
                int p2 = precedence(o2); // 栈顶运算符优先级
                if ((!rightAssoc(c) && p1 <= p2) || (rightAssoc(c) && p1 < p2)) { // 出栈条件：当前运算符为左结合且优先级不高于栈顶，或为右结合且优先级低于栈顶
                    out.push_back(o2); ops.pop_back(); // 弹出栈顶运算符加入输出
                    continue; // 继续检查下一个栈顶运算符
                }
                break; // 否则停止弹出
            }
            ops.push_back(c); // 当前运算符入栈
            continue;
        }
        // 其它字符直接忽略或可扩展为多位数支持
    }
    while (!ops.empty()) { out.push_back(topOp()); ops.pop_back(); } // 弹出剩余运算符
    return out;
}

double evalInfixExpr(const std::string &infix) {
    std::string postfix = infixToPostfix(infix); // 先转换为后缀表达式
    return evalPostfixExpr(postfix); // 然后计算后缀表达式值
}


// 递归求值
double evalExprTree(ExprNode* root) {
    if (!root) return 0.0; // 空节点返回0
    // 叶节点直接返回数字值
    if (!root->left && !root->right) {
        if (std::isdigit((unsigned char)root->val)) return double(root->val - '0'); // 假设操作数为单字符数字（题目要求）
        throw std::runtime_error("叶节点不是数字"); // 错误处理
    }
    double a = evalExprTree(root->left); // 递归求左子树值
    double b = evalExprTree(root->right); // 递归求右子树值
    char op = root->val; // 当前节点运算符
    switch (op) { // 根据运算符进行计算
        case '+': return a + b;
        case '-': return a - b;
        case '*': return a * b;
        case '^': return std::pow(a, b); // 幂运算
        case '/': return (b == 0.0 ? 0.0 : a / b);
        case '%': {
            long long ia = (long long) a;
            long long ib = (long long) b;
            if (ib == 0) return 0.0;
            return double(ia % ib);
        }
        default: return 0.0;
    }
}

// 根据后缀表达式求值
double evalPostfixExpr(const std::string &postfix) {
    std::vector<double> st; // 使用向量作为栈
    for (char c: postfix) { // 遍历后缀表达式字符
        if (isspace((unsigned char)c)) continue; // 跳过空白字符
        if (std::isdigit((unsigned char)c)) st.push_back(double(c - '0')); // 操作数入栈
        else {
            if (st.size() < 2) throw std::runtime_error("后缀表达式格式错误"); // 错误处理
            double b = st.back(); st.pop_back(); // 弹出栈顶两个操作数
            double a = st.back(); st.pop_back();
            double res = 0.0; // 结果变量
            switch (c) { // 根据运算符计算结果
                case '+': res = a + b; break;
                case '-': res = a - b; break;
                case '*': res = a * b; break;
                case '/': res = (b == 0.0 ? 0.0 : a / b); break;
                case '^': res = std::pow(a, b); break; // 幂运算
                case '%': {
                    long long ia = (long long)a;
                    long long ib = (long long)b;
                    if (ib == 0) res = 0.0;
                    else res = double(ia % ib);
                    break;
                }
                default: throw std::runtime_error("未知运算符");
            }
            st.push_back(res); // 结果入栈
        }
    }
    if (st.empty()) return 0.0; // 空栈返回0
    return st.back(); // 返回栈顶结果
}

#include "poly.h" // 包含多项式类的定义
#include <sstream> // 用于字符串流操作
#include <iomanip> // 用于格式化输出
#include <cmath> // 用于数学函数，如 fabs

#include <unordered_map> // 用于哈希表实现多项式乘法
#include <vector> // 用于存储中间结果
#include <algorithm> // 用于排序

Polynomial::Polynomial() : head(nullptr) {} // 构造函数，初始化头指针为空

Polynomial::~Polynomial() { clear(); } // 析构函数，释放内存

// 清空多项式，释放内存
void Polynomial::clear() {
    Term* cur = head;
    while (cur) {
        Term* t = cur;
        cur = cur->next;
        delete t;
    } // 删除所有项
    head = nullptr; // 清空头指针
}

// 判断多项式是否为零多项式
bool Polynomial::isZero() const {
    return head == nullptr;
}

void Polynomial::insertTerm(double coeff, int exp) {
    if (coeff == 0.0) return;// 忽略系数为0的项

    // 插入保持降序，并合并同类项
    if (!head || exp > head->exp) { //如果插入项指数大于头项指数
        Term* t = new Term(coeff, exp); // 创建新项
        t->next = head; // 新项指向原头项
        head = t; // 更新头指针
        return;
    }

    Term* prev = nullptr; // 记录前一个节点
    Term* cur = head; // 从头开始遍历
    while (cur && cur->exp > exp) {
        prev = cur; // 更新前一个节点
        cur = cur->next; // 移动到下一个节点
    }

    if (cur && cur->exp == exp) { // 找到同类项，合并系数
        cur->coeff += coeff;
        if (std::fabs(cur->coeff) < 1e-12) { // 合并后系数为0则删除该项
            if (!prev) {
                head = cur->next;
            } else {
                prev->next = cur->next;
            }
            delete cur;
        }
    } else { //没有找到同类项，插入新项
        Term* t = new Term(coeff, exp);
        if (!prev) {
            t->next = head;
            head = t;
        } else {
            t->next = prev->next;
            prev->next = t;
        }
    }
}

// 返回导数多项式
Polynomial Polynomial::derivative() const {
    Polynomial res; // 初始化结果多项式
    Term* cur = head; 
    while (cur) {
        // 支持负指数：对任意 exp 计算 coeff*exp，exp-1
        double newCoeff = cur->coeff * cur->exp;
        int newExp = cur->exp - 1;
        // insertTerm 会忽略系数为 0 的项（例如常数项）
        if (std::fabs(newCoeff) > 1e-12) {
            res.insertTerm(newCoeff, newExp);
        }
        cur = cur->next;
    }
    return res;
}

// 返回格式化字符串（系数保留两位小数），空多项式返回 "0"
std::string Polynomial::toString() const {
    if (!head) return "0"; 

    std::ostringstream oss; // 用于构建格式化字符串
    oss << std::fixed << std::setprecision(2); // 设置浮点数格式为固定小数点，保留两位小数
    Term* cur = head;
    bool first = true;
    while (cur) {
        double c = cur->coeff;
        int e = cur->exp;
        if (std::fabs(c) < 1e-12) { cur = cur->next; continue; } // 忽略系数为0的项

        // 符号
        if (first) {
            if (c < 0) oss << "-";
        } else {
            oss << (c < 0 ? "-" : "+");
        }

        // 系数绝对值
        double ac = std::fabs(c);
        // 输出系数（始终保留两位）
        oss << std::fixed << std::setprecision(2) << ac;

        // 输出 x 部分
        if (e == 0) {
            // 常数项，已经输出系数
        } else if (e == 1) {
            oss << "x";
        } else {
            oss << "x^" << e;
        }

        first = false;
        cur = cur->next;
    }
    return oss.str();
}

// 多项式乘法：不使用额外库，直接逐项相乘并插入结果（insertTerm 负责合并）
Polynomial Polynomial::multiplyNaive(const Polynomial& other) const {
    Polynomial res;
    if (!head || !other.head) return res;
    for (Term* a = head; a; a = a->next) {
        for (Term* b = other.head; b; b = b->next) {
            double c = a->coeff * b->coeff;
            int e = a->exp + b->exp;
            res.insertTerm(c, e); // insertTerm 合并同类项并忽略系数为0的项
        }
    }
    return res;
} //时间复杂度 O(m*n*log(mn))

// 多项式乘法：使用哈希表累加结果，再排序插入，效率更高
Polynomial Polynomial::multiplyHash(const Polynomial& other) const { 
    Polynomial res; // 结果多项式
    if (!head || !other.head) return res; // 任一多项式为零则结果为零
    std::unordered_map<int,double> acc; // 哈希表用于累加同类项
    // 逐项相乘并累加到哈希表
    for (Term* a = head; a; a = a->next) {
        for (Term* b = other.head; b; b = b->next) {
            acc[a->exp + b->exp] += a->coeff * b->coeff;
        }
    }
    const double EPS = 1e-12; // 忽略系数接近0的项
    std::vector<std::pair<int,double>> items; // 存储非零项以便排序
    for (auto &p : acc) if (std::fabs(p.second) > EPS) items.emplace_back(p.first, p.second);  // 仅保留系数非零项，emplace_back：在容器末尾原地构造新元素
    std::sort(items.begin(), items.end(), [](auto &l, auto &r){ return l.first > r.first; }); // 按指数降序排序
    for (auto &it : items) res.insertTerm(it.second, it.first); // 插入结果多项式
    return res; // 返回结果多项式 
} //时间复杂度 O(m*n)

// 深拷贝链表
Term* Polynomial::cloneList() const {
    if (!head) return nullptr;
    Term* src = head;
    Term* dstHead = new Term(src->coeff, src->exp);
    Term* dst = dstHead;
    src = src->next;
    while (src) {
        dst->next = new Term(src->coeff, src->exp);
        dst = dst->next;
        src = src->next;
    }
    dst->next = nullptr;
    return dstHead;
}

// 拷贝构造
Polynomial::Polynomial(const Polynomial& other) : head(nullptr) {
    head = other.cloneList();
}

// 拷贝赋值
Polynomial& Polynomial::operator=(const Polynomial& other) {
    if (this == &other) return *this;
    clear();
    head = other.cloneList();
    return *this;
}

// 移动构造
Polynomial::Polynomial(Polynomial&& other) noexcept : head(other.head) {
    other.head = nullptr;
}

// 移动赋值
Polynomial& Polynomial::operator=(Polynomial&& other) noexcept {
    if (this == &other) return *this;
    clear();
    head = other.head;
    other.head = nullptr;
    return *this;
}
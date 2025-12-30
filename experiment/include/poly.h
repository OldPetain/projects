#ifndef POLY_H
#define POLY_H

#include <string>

struct Term {
    double coeff;
    int exp;
    Term* next;
    Term(double c, int e) : coeff(c), exp(e), next(nullptr) {}
}; // 多项式项结构体, 包含系数coeff、指数exp和指向下一项的指针next

class Polynomial {
private:
    Term* head; // 按指数降序排列
    void clear();

    // 深拷贝辅助
    Term* cloneList() const;
public:
    Polynomial();
    ~Polynomial();

    // 拷贝与移动语义（避免双重释放）
    Polynomial(const Polynomial& other);            // 拷贝构造
    Polynomial& operator=(const Polynomial& other); // 拷贝赋值
    Polynomial(Polynomial&& other) noexcept;        // 移动构造
    Polynomial& operator=(Polynomial&& other) noexcept; // 移动赋值

    // 插入项（合并同类项，保持降序），若系数为0则忽略或删除
    void insertTerm(double coeff, int exp);

    // 返回导数多项式
    Polynomial derivative() const;

    // 两种乘法接口
    Polynomial multiplyNaive(const Polynomial& other) const; // 逐项相乘，直接插入（不额外容器）
    Polynomial multiplyHash(const Polynomial& other) const;  // 使用哈希累加再构造结果（更快）

    // 返回格式化字符串（系数保留两位小数），空多项式返回 "0"
    std::string toString() const;

    bool isZero() const;
};

#endif
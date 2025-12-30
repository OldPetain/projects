#include "array.h"

// 构造函数
void Array::insert(int val) {
    data.push_back(val);
}
// 直接插入排序
void Array::directInsertSort() {
    int n = data.size();
    for (int i = 1; i < n; ++i) {
        int key = data[i];
        int j = i - 1;
        while (j >= 0 && data[j] > key) {
            data[j + 1] = data[j];
            --j;
        }
        data[j + 1] = key;
    }
}
// 直接选择排序
void Array::directSelectSort() {
    int n = data.size();
    for (int i = 0; i < n - 1; ++i) {
        int minIdx = i;
        for (int j = i + 1; j < n; ++j) {
            if (data[j] < data[minIdx]) {
                minIdx = j;
            }
        }
        std::swap(data[i], data[minIdx]);
    }
}
// 冒泡排序
void Array::bubbleSort() {
    int n = data.size();
    for (int i = 0; i < n -1; ++i) {
        for (int j = 0; j < n - i -1; ++j) {
            if (data[j] > data[j+1]) {
                std::swap(data[j], data[j+1]);
            }
        }
    }
}
// 快速排序
int Array::sequentialSearch(int val) const {
    for (int i = 0; i < (int)data.size(); ++i) {
        if (data[i] == val) return i;
    }
    return -1;  // 找不到返回-1
}

// 显示数组
void Array::display() const {
    for (int v : data) {
        std::cout << v << " ";
    }
    std::cout << std::endl;
}

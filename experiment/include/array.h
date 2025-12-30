#ifndef ARRAY_H
#define ARRAY_H

#include <iostream>
#include <vector>

class Array {
private:
    std::vector<int> data;

public:
    Array() = default;
    Array(const std::vector<int>& v) : data(v) {}

    void insert(int val);
    void directInsertSort();
    void directSelectSort();
    void bubbleSort();
    int sequentialSearch(int val) const;

    void display() const;
};

#endif

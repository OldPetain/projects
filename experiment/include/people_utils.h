#ifndef PEOPLE_UTILS_H
#define PEOPLE_UTILS_H

#include "people.h"
#include <vector>
#include <algorithm>

// 按编号排序
void sortPeopleById(std::vector<People>& arr) {
    std::sort(arr.begin(), arr.end());
}

// 顺序查找编号
int searchPeopleById(const std::vector<People>& arr, int id) {
    for (size_t i = 0; i < arr.size(); ++i) {
        if (arr[i].id == id) return (int)i;
    }
    return -1;
}

#endif

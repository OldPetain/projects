// tensor.cpp
#include "tensor.hpp"
#include <cstdlib>
#include <ctime>
#include <stdexcept>

Tensor2D::Tensor2D(size_t rows, size_t cols, float init_val)
    : rows_(rows), cols_(cols), data_(rows, std::vector<float>(cols, init_val)) {
    std::srand(static_cast<unsigned>(std::time(nullptr)));  // 用于 randomize
}

float& Tensor2D::at(size_t row, size_t col) {
    if (row >= rows_ || col >= cols_)
        throw std::out_of_range("Tensor2D::at() out of range");
    return data_[row][col];
}

float Tensor2D::at(size_t row, size_t col) const {
    if (row >= rows_ || col >= cols_)
        throw std::out_of_range("Tensor2D::at() const out of range");
    return data_[row][col];
}

size_t Tensor2D::rows() const {
    return rows_;
}

size_t Tensor2D::cols() const {
    return cols_;
}

void Tensor2D::fill(float val) {
    for (auto& row : data_)
        std::fill(row.begin(), row.end(), val);
}

void Tensor2D::randomize() {
    for (auto& row : data_)
        for (auto& val : row)
            val = static_cast<float>(std::rand()) / RAND_MAX * 0.1f - 0.05f;  // -0.05 ~ 0.05
}

std::vector<float>& Tensor2D::operator[](size_t row) {
    return data_[row];
}

const std::vector<float>& Tensor2D::operator[](size_t row) const {
    return data_[row];
}
// tensor.hpp
#pragma once

#include <vector>
#include <cstdlib>
#include <ctime>
#include <stdexcept>

class Tensor2D {
public:
    Tensor2D(size_t rows, size_t cols, float init_val = 0.0f);

    float& at(size_t row, size_t col);
    float at(size_t row, size_t col) const;

    size_t rows() const;
    size_t cols() const;

    void fill(float val);
    void randomize();

    std::vector<float>& operator[](size_t row);
    const std::vector<float>& operator[](size_t row) const;

private:
    size_t rows_;
    size_t cols_;
    std::vector<std::vector<float>> data_;
};
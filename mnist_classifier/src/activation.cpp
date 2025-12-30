// activation.cpp
#include "activation.hpp"
#include <cmath>

namespace Activation {

    // sigmoid 激活函数（y = 1 / (1 + exp(-x))）
float sigmoid(float x) {
    return 1.0f / (1.0f + std::exp(-x));
}

float sigmoid_derivative(float x) {
    float s = sigmoid(x);
    return s * (1 - s);
}
    // relu 激活函数（y = max(0, x)）
float relu(float x) {
    return x > 0 ? x : 0;
}

float relu_derivative(float x) {
    return x > 0 ? 1.0f : 0.0f;
}

    // 向量版本
std::vector<float> apply_sigmoid(const std::vector<float>& input) {
    std::vector<float> output(input.size());
    for (size_t i = 0; i < input.size(); ++i) {
        output[i] = sigmoid(input[i]);
    }
    return output;
}

std::vector<float> apply_sigmoid_deriv(const std::vector<float>& input) {
    std::vector<float> output(input.size());
    for (size_t i = 0; i < input.size(); ++i) {
        output[i] = sigmoid_derivative(input[i]);
    }
    return output;
}

std::vector<float> apply_relu(const std::vector<float>& input) {
    std::vector<float> output(input.size());
    for (size_t i = 0; i < input.size(); ++i) {
        output[i] = relu(input[i]);
    }
    return output;
}

std::vector<float> apply_relu_deriv(const std::vector<float>& input) {
    std::vector<float> output(input.size());
    for (size_t i = 0; i < input.size(); ++i) {
        output[i] = relu_derivative(input[i]);
    }
    return output;
}

}
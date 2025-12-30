// loss.cpp
#include "loss.hpp"
#include <cmath>
#include <stdexcept>

namespace Loss {

    float cross_entropy(const std::vector<float>& prediction, const std::vector<float>& target) {
        // 输入检查
        if (prediction.size() != target.size())
            throw std::runtime_error("Prediction and target size mismatch in cross_entropy");

        // 准备返回的损失
        float loss = 0.0f;
        for (size_t i = 0; i < prediction.size(); ++i) {
            float pred = std::max(1e-7f, std::min(1.0f - 1e-7f, prediction[i])); // 防止 log(0)
            loss -= target[i] * std::log(pred);
        }
        return loss;
    }

    std::vector<float> cross_entropy_derivative(const std::vector<float>& prediction, const std::vector<float>& target) {
        // 输入检查
        if (prediction.size() != target.size())
            throw std::runtime_error("Prediction and target size mismatch in derivative");
        // 准备返回的梯度
        std::vector<float> grad;
        grad.reserve(prediction.size());

        // 计算梯度
        for (size_t i = 0; i < prediction.size(); ++i) {
            float pred = std::max(1e-7f, std::min(1.0f - 1e-7f, prediction[i])); // 避免除0
            grad.push_back(-target[i] / pred);
        }

        return grad;
    }

}
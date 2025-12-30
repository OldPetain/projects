// mlp.hpp
#pragma once

#include <vector>
#include <string>
#include <memory>
#include "layer.hpp"
#include "optimizer.hpp"

class MLP {
public:
    MLP(const std::vector<int>& layers,
        const std::string& activation = "relu",
        const std::string& optimizer_type = "sgd");

    // 前向传播
    std::vector<float> forward(const std::vector<float>& input);

    // 反向传播 + 权重更新 (支持批处理)
    void backward(const std::vector<std::vector<float>>& inputs,
                  const std::vector<std::vector<float>>& targets,
                  float learning_rate);

private:
    std::vector<std::unique_ptr<Layer>> layers_;
    std::unique_ptr<Optimizer> optimizer_;

    std::vector<float> softmax(const std::vector<float>& x);
    float cross_entropy(const std::vector<float>& y_pred,
                        const std::vector<float>& y_true);
};
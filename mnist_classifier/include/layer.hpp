// layer.hpp
#pragma once

#include <vector>
#include <string>
#include <functional>
#include "optimizer.hpp"

class Layer {
public:
    Layer(int input_size, int output_size,
          const std::string& activation_name,
          Optimizer* optimizer);

    std::vector<float> forward(const std::vector<float>& input);
    std::vector<float> backward(const std::vector<float>& d_out); // 单样本 backward (保留)

    // 支持批处理的前向传播
    std::vector<float> forward_batch(const std::vector<float>& input);
    std::vector<float> get_z() const { return z_; }

    // 支持批处理的后向传播
    std::vector<float> backward_batch(const std::vector<float>& input, const std::vector<float>& d_out, const std::vector<float>& z);

    void update(const std::vector<float>& d_weights, const std::vector<float>& d_biases, float learning_rate);

    int input_size() const { return input_size_; }
    int output_size() const { return output_size_; }

private:
    int input_size_;
    int output_size_;
    std::vector<float> weights_;
    std::vector<float> biases_;
    std::vector<float> input_;
    std::vector<float> output_;
    std::vector<float> z_;
    std::function<float(float)> activation_;
    std::function<float(float)> activation_derivative_;
    Optimizer* optimizer_;
};
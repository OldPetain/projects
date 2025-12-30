// layer.cpp
#include "layer.hpp"
#include <random>
#include <cmath>
#include <stdexcept>
#include "activation.hpp"
#include <numeric> // for std::inner_product

Layer::Layer(int input_size, int output_size,
             const std::string &activation_name,
             Optimizer *optimizer)
    : input_size_(input_size), output_size_(output_size), optimizer_(optimizer)
{

    weights_.resize(input_size * output_size);
    biases_.resize(output_size, 0.0f);

    // Xavier 初始化权重
    std::mt19937 rng(std::random_device{}());
    std::normal_distribution<float> dist(0.0f, std::sqrt(2.0f / (input_size + output_size)));
    for (auto &w : weights_)
    {
        w = dist(rng);
    }

    if (activation_name == "relu")
    {
        activation_ = Activation::relu;
        activation_derivative_ = Activation::relu_derivative;
    }
    else if (activation_name == "sigmoid")
    {
        activation_ = Activation::sigmoid;
        activation_derivative_ = Activation::sigmoid_derivative;
    }
    else
    {
        throw std::runtime_error("Unsupported activation: " + activation_name);
    }
}

std::vector<float> Layer::forward(const std::vector<float> &input)
{
    input_ = input;
    output_.resize(output_size_);
    z_.resize(output_size_);

    for (int j = 0; j < output_size_; ++j)
    {
        float z = biases_[j];
        for (int i = 0; i < input_size_; ++i)
        {
            z += input[i] * weights_[j * input_size_ + i];
        }
        z_[j] = z;
        output_[j] = activation_(z);
    }

    return output_;
}

std::vector<float> Layer::forward_batch(const std::vector<float>& input) {
    input_ = input;
    output_.resize(output_size_);
    z_.resize(output_size_);

    for (int j = 0; j < output_size_; ++j) {
        float z = biases_[j];
        for (int i = 0; i < input_size_; ++i) {
            z += input[i] * weights_[j * input_size_ + i];
        }
        z_[j] = z;
        output_[j] = activation_(z);
    }
    return output_;
}

std::vector<float> Layer::backward(const std::vector<float> &d_out)
{
    std::vector<float> d_input(input_size_, 0.0f);
    std::vector<float> d_weights(weights_.size(), 0.0f);
    std::vector<float> d_biases(output_size_, 0.0f);

    for (int j = 0; j < output_size_; ++j)
    {
        float delta = d_out[j] * activation_derivative_(z_[j]); // 注意这里传入 z_，而非 output_
        d_biases[j] = delta;

        for (int i = 0; i < input_size_; ++i)
        {
            d_weights[j * input_size_ + i] = delta * input_[i];
            d_input[i] += weights_[j * input_size_ + i] * delta;
        }
    }

    optimizer_->update(weights_, d_weights, 1.0f);
    optimizer_->update(biases_, d_biases, 1.0f);

    return d_input;
}

std::vector<float> Layer::backward_batch(const std::vector<float>& input, const std::vector<float>& d_out, const std::vector<float>& z) {
    std::vector<float> d_input(input_size_, 0.0f);
    std::vector<float> d_weights(weights_.size(), 0.0f);
    std::vector<float> d_biases(output_size_, 0.0f);

    for (int j = 0; j < output_size_; ++j) {
        float delta = d_out[j] * activation_derivative_(z[j]);
        d_biases[j] = delta;
        for (int i = 0; i < input_size_; ++i) {
            d_weights[j * input_size_ + i] = delta * input[i];
            d_input[i] += weights_[j * input_size_ + i] * delta;
        }
    }
    return d_input;
}

void Layer::update(const std::vector<float>& d_weights, const std::vector<float>& d_biases, float learning_rate) {
    optimizer_->update(weights_, d_weights, learning_rate);
    optimizer_->update(biases_, d_biases, learning_rate);
}
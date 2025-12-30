// mlp.cpp
#include "mlp.hpp"
#include <cmath>
#include <stdexcept>
#include <iostream>
#include <numeric> // for std::inner_product

MLP::MLP(const std::vector<int>& layer_sizes,
         const std::string& activation,
         const std::string& optimizer_type) {
    if (optimizer_type == "sgd") {
        optimizer_ = std::make_unique<SGD>();
    } else if (optimizer_type == "momentum") {
        optimizer_ = std::make_unique<Momentum>(0.9f);
    } else {
        throw std::runtime_error("Unsupported optimizer: " + optimizer_type);
    }

    for (size_t i = 1; i < layer_sizes.size(); ++i) {
        layers_.emplace_back(std::make_unique<Layer>(
            layer_sizes[i - 1], layer_sizes[i], activation, optimizer_.get()));
    }
}

std::vector<float> MLP::forward(const std::vector<float>& input) {
    std::vector<float> out = input;
    for (auto& layer : layers_) {
        out = layer->forward(out);
    }
    return softmax(out);
}

void MLP::backward(const std::vector<std::vector<float>>& inputs,
                    const std::vector<std::vector<float>>& targets,
                    float learning_rate) {
    if (inputs.empty()) {
        return;
    }

    size_t batch_size = inputs.size();
    std::vector<std::vector<float>> current_inputs = inputs;
    std::vector<std::vector<std::vector<float>>> layer_inputs_history; // Stores the input to each layer for each batch
    std::vector<std::vector<std::vector<float>>> layer_z_values;

    // 前向传播 (存储每层的输入 and z 值)
    layer_inputs_history.push_back(inputs); // Input to the first hidden layer
    for (auto& layer : layers_) {
        std::vector<std::vector<float>> next_outputs(batch_size);
        std::vector<std::vector<float>> current_z_values(batch_size, std::vector<float>(layer->output_size()));
        for (size_t i = 0; i < batch_size; ++i) {
            next_outputs[i] = layer->forward_batch(current_inputs[i]);
            current_z_values[i] = layer->get_z();
        }
        layer_z_values.push_back(current_z_values);
        current_inputs = next_outputs;
        layer_inputs_history.push_back(current_inputs); // Input to the next layer
    }
    std::vector<std::vector<float>> predictions = current_inputs; // Output of the last layer

    // 反向传播
    std::vector<std::vector<float>> delta = predictions; // Initial delta for the output layer
    for (size_t i = 0; i < batch_size; ++i) {
        for (size_t j = 0; j < delta[i].size(); ++j) {
            delta[i][j] -= targets[i][j]; // Gradient of cross-entropy loss
        }
    }

    for (int i = layers_.size() - 1; i >= 0; --i) {
        const std::vector<std::vector<float>>& current_layer_input = layer_inputs_history[i];
        const std::vector<std::vector<float>>& current_z_batch = layer_z_values[i];
        std::vector<std::vector<float>> prev_delta(batch_size, std::vector<float>(layers_[i]->input_size()));


        // Accumulate gradients for the batch
        std::vector<float> accumulated_d_weights(layers_[i]->input_size() * layers_[i]->output_size(), 0.0f);
        std::vector<float> accumulated_d_biases(layers_[i]->output_size(), 0.0f);

        for (size_t batch_idx = 0; batch_idx < batch_size; ++batch_idx) {
            const auto& input = current_layer_input[batch_idx];
            const auto& d_out = delta[batch_idx];
            const auto& z = current_z_batch[batch_idx];
            std::vector<float> d_input = layers_[i]->backward_batch(input, d_out, z);

            // Accumulate gradients
            for (size_t j = 0; j < layers_[i]->output_size(); ++j) {
                accumulated_d_biases[j] += d_out[j];
                for (size_t k = 0; k < layers_[i]->input_size(); ++k) {
                    accumulated_d_weights[j * layers_[i]->input_size() + k] += d_out[j] * input[k];
                }
            }
            prev_delta[batch_idx] = d_input;
        }

        // Apply averaged gradients
        float scale = 1.0f / batch_size;
        std::vector<float> avg_d_weights = accumulated_d_weights;
        for (auto& dw : avg_d_weights) dw *= scale;
        std::vector<float> avg_d_biases = accumulated_d_biases;
        for (auto& db : avg_d_biases) db *= scale;

        layers_[i]->update(avg_d_weights, avg_d_biases, learning_rate);
        delta = prev_delta;
    }
}

std::vector<float> MLP::softmax(const std::vector<float>& x) {
    std::vector<float> exp_x(x.size());
    float max_val = *std::max_element(x.begin(), x.end());
    float sum = 0.0f;
    for (size_t i = 0; i < x.size(); ++i) {
        exp_x[i] = std::exp(x[i] - max_val);
        sum += exp_x[i];
    }
    for (auto& val : exp_x) {
        val /= sum;
    }
    return exp_x;
}

float MLP::cross_entropy(const std::vector<float>& y_pred,
                            const std::vector<float>& y_true) {
    float loss = 0.0f;
    for (size_t i = 0; i < y_pred.size(); ++i) {
        loss -= y_true[i] * std::log(y_pred[i] + 1e-8f);
    }
    return loss;
}









//cnn_model.hpp
#pragma once
#include <torch/torch.h>

struct CNNImpl : public torch::nn::Module {
    CNNImpl();

    torch::Tensor forward(torch::Tensor x);

    torch::nn::Conv2d conv1{nullptr};
    torch::nn::Conv2d conv2{nullptr};
    torch::nn::Linear fc1{nullptr}, fc2{nullptr};
};

TORCH_MODULE(CNN);

// cnn_model.cpp
#include "cnn_model.hpp"

CNNImpl::CNNImpl() {
    // 定义网络结构
    conv1 = register_module("conv1", torch::nn::Conv2d(1, 10, 5));  // 输出通道10, 卷积核5x5
    conv2 = register_module("conv2", torch::nn::Conv2d(10, 20, 5)); // 输出通道20, 卷积核5x5
    // 定义全连接层
    fc1 = register_module("fc1", torch::nn::Linear(320, 50)); // 输入320, 输出50
    fc2 = register_module("fc2", torch::nn::Linear(50, 10)); // 输入50, 输出10
}

torch::Tensor CNNImpl::forward(torch::Tensor x) {
    x = torch::relu(torch::max_pool2d(conv1->forward(x), 2));// 卷积层1, 池化层1
    x = torch::relu(torch::max_pool2d(conv2->forward(x), 2));// 卷积层2, 池化层2
    x = x.view({x.size(0), -1}); // 全连接层1, 展开
    x = torch::relu(fc1->forward(x)); // 全连接层1, 激活函数
    x = fc2->forward(x); // 全连接层2, 输出
    return torch::log_softmax(x, 1); // 输出层, 输出（log_softmax：softmax的对数形式）
}

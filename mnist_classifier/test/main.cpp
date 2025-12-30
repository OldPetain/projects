// main_cnn.cpp
#include "mnist_reader.hpp"
#include "metrics.hpp"
#include "cnn_model.hpp"
#include <torch/torch.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>

int main() {
    torch::Device device(torch::cuda::is_available() ? torch::kCUDA : torch::kCPU);
    std::cout << "当前设备: " << (device.is_cuda() ? "CUDA (GPU)" : "CPU") << std::endl;

    // 1. 加载并处理数据
    std::cout << "正在加载 MNIST 训练与测试数据..." << std::endl;
    auto train_images_u8 = MNISTReader::load_images("data/train-images-idx3-ubyte");
    auto train_labels = MNISTReader::load_labels("data/train-labels-idx1-ubyte");
    auto test_images_u8 = MNISTReader::load_images("data/t10k-images-idx3-ubyte");
    auto test_labels = MNISTReader::load_labels("data/t10k-labels-idx1-ubyte");

    std::cout << "共加载训练图像 " << train_images_u8.size() << " 张，测试图像 " << test_images_u8.size() << " 张。" << std::endl;

    auto train_images = MNISTReader::normalize_images(train_images_u8);
    auto test_images = MNISTReader::normalize_images(test_images_u8);

    auto to_tensor = [](const std::vector<std::vector<float>>& images) {
        torch::Tensor tensor = torch::zeros({static_cast<long>(images.size()), 1, 28, 28});
        for (size_t i = 0; i < images.size(); ++i) {
            tensor[i][0] = torch::from_blob((void*)images[i].data(), {28, 28}, torch::kFloat).clone();
        }
        return tensor;
    };

    auto train_x = to_tensor(train_images).to(device);
    auto train_y = torch::from_blob(train_labels.data(), {static_cast<long>(train_labels.size())}, torch::kByte).to(torch::kLong).to(device);
    auto test_x = to_tensor(test_images).to(device);
    auto test_y = torch::from_blob(test_labels.data(), {static_cast<long>(test_labels.size())}, torch::kByte).to(torch::kLong).to(device);

    // 2. 模型定义
    CNN model;
    model->to(device);

    torch::optim::SGD optimizer(model->parameters(), torch::optim::SGDOptions(0.01).momentum(0.5));

    // 3. 训练过程
    int epochs = 10, batch_size = 64;
    std::cout << "\n=== 开始训练，共 " << epochs << " 轮 ===\n" << std::endl;

    for (int epoch = 1; epoch <= epochs; ++epoch) {
        float running_loss = 0.0;
        model->train();
        int batch_count = 0;

        for (size_t start = 0; start < train_x.size(0); start += batch_size) {
            size_t end = std::min(start + batch_size, (size_t)train_x.size(0));
            auto batch_x = train_x.slice(0, start, end);
            auto batch_y = train_y.slice(0, start, end);

            optimizer.zero_grad();
            auto output = model->forward(batch_x);
            auto loss = torch::nll_loss(output, batch_y);
            loss.backward();
            optimizer.step();

            running_loss += loss.item<float>();
            ++batch_count;
        }

        std::cout << "第 [" << epoch << "/" << epochs << "] 轮训练完成，平均损失: " << (running_loss / batch_count) << std::endl;
    }
    
    // 保存模型
    std::cout << "\n=== 训练完成，正在保存模型 ===" << std::endl;
    torch::save(model, "model/cnn_model.pt");
    std::cout << "模型已保存至文件: model/cnn_model.pt" << std::endl;

    // 4. 模型测试
    std::cout << "\n=== 开始模型测试 ===" << std::endl;
    model->eval();
    torch::NoGradGuard no_grad;

    auto output = model->forward(test_x);  // [N, 10]
    auto predicted = output.argmax(1).cpu();
    std::vector<uint8_t> preds(predicted.data_ptr<int64_t>(), predicted.data_ptr<int64_t>() + predicted.size(0));

    std::vector<std::vector<float>> probs;
    for (int i = 0; i < output.size(0); ++i) {
        std::vector<float> prob(output.size(1));
        std::memcpy(prob.data(), output[i].exp().cpu().data_ptr<float>(), sizeof(float) * prob.size());
        probs.push_back(prob);
    }
    std::cout << "测试完成，共预测 " << preds.size() << " 张图像。" << std::endl;

    // 5. 输出评估报告
    std::cout << "\n=== 分类性能评估报告 ===" << std::endl;
    auto report = evaluate_classification(test_labels, preds, 10);
    std::cout << "准确率(Accuracy): " << report.accuracy << std::endl;
    std::cout << "宏平均精确率(Precision macro): " << report.precision_macro << std::endl;
    std::cout << "宏平均召回率(Recall macro): " << report.recall_macro << std::endl;
    std::cout << "宏平均F1值(F1 score macro): " << report.f1_macro << std::endl;

    // 6. 保存预测概率
    std::string out_path = "output/cnn_prob_output.csv";
    export_probabilities_to_csv(test_labels, probs, "cnn");
    
    return 0;
}
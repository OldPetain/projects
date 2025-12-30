// main_cnn_svm.cpp
#include "mnist_reader.hpp"
#include "cnn_model.hpp"
#include "svm_classifier.hpp"
#include "metrics.hpp"

#include <torch/torch.h>
#include <iostream>
#include <filesystem>

// 将 std::vector<std::vector<float>> 转为 Tensor
torch::Tensor to_tensor(const std::vector<std::vector<float>>& images) {
    auto n = images.size();
    auto flat = torch::empty({static_cast<long>(n), 28 * 28}, torch::kFloat32);
    for (size_t i = 0; i < n; ++i) {
        std::memcpy(flat[i].data_ptr<float>(), images[i].data(), 28 * 28 * sizeof(float));
    }
    return flat.view({static_cast<long>(n), 1, 28, 28});
}

// 提取 CNN fc1 特征
std::vector<std::vector<float>> extract_fc1_features(CNN& model, const torch::Tensor& images, torch::Device device) {
    model->eval();
    torch::NoGradGuard no_grad;

    auto x = images.to(device);
    x = torch::relu(torch::max_pool2d(model->conv1->forward(x), 2));
    x = torch::relu(torch::max_pool2d(model->conv2->forward(x), 2));
    x = x.view({x.size(0), -1});
    x = torch::relu(model->fc1->forward(x));
    x = x.cpu();

    std::vector<std::vector<float>> features(x.size(0));
    for (int i = 0; i < x.size(0); ++i) {
        features[i].assign(x[i].data_ptr<float>(), x[i].data_ptr<float>() + x.size(1));
    }
    return features;
}

int main() {
    try {
        torch::Device device(torch::cuda::is_available() ? torch::kCUDA : torch::kCPU);
        std::cout << "当前设备: " << device << std::endl;

        // 手动加载和处理数据
        std::cout << "加载训练图像与标签..." << std::endl;
        auto train_raw = MNISTReader::load_images("data/train-images-idx3-ubyte");
        auto train_labels = MNISTReader::load_labels("data/train-labels-idx1-ubyte");
        auto train_images = MNISTReader::normalize_images(train_raw);

        std::cout << "加载测试图像与标签..." << std::endl;
        auto test_raw = MNISTReader::load_images("data/t10k-images-idx3-ubyte");
        auto test_labels = MNISTReader::load_labels("data/t10k-labels-idx1-ubyte");
        auto test_images = MNISTReader::normalize_images(test_raw);

        std::cout << "训练图像数: " << train_images.size() << ", 测试图像数: " << test_images.size() << std::endl;

        // 转为 tensor
        auto train_x = to_tensor(train_images);
        auto test_x = to_tensor(test_images);
        std::cout << "train_x shape: " << train_x.sizes() << ", test_x shape: " << test_x.sizes() << std::endl;

        // 加载模型
        CNN model;
        model->to(device);
        if (!std::filesystem::exists("model/cnn_model.pt")) {
            std::cerr << "未找到模型文件: model/cnn_model.pt" << std::endl;
            return 1;
        }
        torch::load(model, "model/cnn_model.pt");

        std::cout << "提取 CNN 特征..." << std::endl;
        auto train_features = extract_fc1_features(model, train_x, device);
        auto test_features = extract_fc1_features(model, test_x, device);

        std::cout << "训练 SVM..." << std::endl;
        SVMClassifier svm;
        svm.train(train_features, train_labels);

        std::vector<uint8_t> preds;// 预测结果
        std::vector<std::vector<float>> probs; // 预测概率
        for (const auto& feat : test_features) {
            preds.push_back(svm.predict(feat));
            probs.push_back(svm.predict_proba(feat));
        }

        auto report = evaluate_classification(test_labels, preds, 10);
        std::cout << "\n--- CNN 特征 + SVM 分类结果 ---" << std::endl;
        std::cout << "准确率 (Accuracy): " << report.accuracy << std::endl;
        std::cout << "宏平均 Precision: " << report.precision_macro << std::endl;
        std::cout << "宏平均 Recall: " << report.recall_macro << std::endl;
        std::cout << "宏平均 F1-score: " << report.f1_macro << std::endl;

        export_probabilities_to_csv(test_labels, probs, "cnn_svm");

    } catch (const std::exception& e) {
        std::cerr << "发生异常: " << e.what() << std::endl;
    }

    return 0;
}

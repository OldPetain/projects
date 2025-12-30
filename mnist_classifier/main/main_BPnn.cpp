//main_BPnn.cpp
#include "mnist_reader.hpp"
#include "mlp.hpp"
#include "metrics.hpp" // 确保包含了 export_probabilities_to_csv 声明
#include <iostream>
#include <vector>
#include <numeric>
#include <algorithm>
#include <random>

std::vector<float> to_one_hot(uint8_t label, int num_classes = 10) {
    std::vector<float> one_hot(num_classes, 0.0f);
    one_hot[label] = 1.0f;
    return one_hot;
}

int main() {
    auto images_train = MNISTReader::load_images("data/train-images-idx3-ubyte");
    auto labels_train = MNISTReader::load_labels("data/train-labels-idx1-ubyte");
    auto images_test = MNISTReader::load_images("data/t10k-images-idx3-ubyte");
    auto labels_test = MNISTReader::load_labels("data/t10k-labels-idx1-ubyte");

    auto normalized_images_train = MNISTReader::normalize_images(images_train);
    auto normalized_images_test = MNISTReader::normalize_images(images_test);

    std::cout << "Loaded " << images_train.size() << " training images.\n";

    std::vector<int> layers = {784, 128, 10};
    MLP mlp(layers, "relu", "momentum");

    int epochs = 10;
    float learning_rate = 0.01f;
    int batch_size = 32;
    size_t num_train_samples = normalized_images_train.size();

    std::random_device rd;
    std::mt19937 gen(rd());

    for (int epoch = 0; epoch < epochs; ++epoch) {
        std::cout << "Epoch " << (epoch + 1) << "/" << epochs << "\n";
        std::vector<size_t> indices(num_train_samples);
        std::iota(indices.begin(), indices.end(), 0);
        std::shuffle(indices.begin(), indices.end(), gen);

        for (size_t i = 0; i < num_train_samples; i += batch_size) {
            size_t end_index = std::min(i + batch_size, num_train_samples);
            size_t current_batch_size = end_index - i;

            std::vector<std::vector<float>> batch_inputs(current_batch_size);
            std::vector<std::vector<float>> batch_targets(current_batch_size);

            for (size_t j = 0; j < current_batch_size; ++j) {
                size_t index = indices[i + j];
                batch_inputs[j] = normalized_images_train[index];
                batch_targets[j] = to_one_hot(labels_train[index]);
            }

            mlp.backward(batch_inputs, batch_targets, learning_rate);

            if (i % 1000 == 0) {
                std::cout << "Processed " << i << " samples in this epoch\n";
            }
        }
    }

    // 测试集预测 + 保存每类概率
    std::vector<uint8_t> mlp_predictions; // 保存预测标签
    std::vector<std::vector<float>> mlp_probabilities; // 保存每类概率向量

    for (const auto& img : normalized_images_test) {
        auto output = mlp.forward(img); // 每类概率
        mlp_probabilities.push_back(output); // 保存概率向量

        auto max_iter = std::max_element(output.begin(), output.end());
        uint8_t pred_label = static_cast<uint8_t>(std::distance(output.begin(), max_iter));
        mlp_predictions.push_back(pred_label);
    }

    // 评估 & 输出报告
    auto mlp_report = evaluate_classification(labels_test, mlp_predictions, 10);
    std::cout << "MLP Accuracy: " << mlp_report.accuracy << "\n";
    std::cout << "MLP Precision (macro): " << mlp_report.precision_macro << "\n";
    std::cout << "MLP Recall (macro): " << mlp_report.recall_macro << "\n";
    std::cout << "MLP F1 Score (macro): " << mlp_report.f1_macro << "\n";

    // 保存CSV文件用于Python绘图
    export_probabilities_to_csv(labels_test, mlp_probabilities, "mlp");

    return 0;
}

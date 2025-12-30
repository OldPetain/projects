// main_knn.cpp
#include "mnist_reader.hpp"
#include "knn_classifier.hpp"
#include "metrics.hpp"
#include <iostream>
#include <vector>
#include <map>
#include <algorithm>  // for std::partial_sort, std::max_element
#include <numeric>    // for std::iota
#include <fstream>

int main() {
    // 加载数据
    auto images_train = MNISTReader::load_images("data/train-images-idx3-ubyte");
    auto labels_train = MNISTReader::load_labels("data/train-labels-idx1-ubyte");
    auto images_test = MNISTReader::load_images("data/t10k-images-idx3-ubyte");
    auto labels_test = MNISTReader::load_labels("data/t10k-labels-idx1-ubyte");

    auto normalized_images_train = MNISTReader::normalize_images(images_train);
    auto normalized_images_test = MNISTReader::normalize_images(images_test);

    std::cout << "Loaded " << images_train.size() << " training images.\n";

    int k = 5;
    KNNClassifier knn(k);
    knn.train(normalized_images_train, labels_train);

    std::vector<uint8_t> predictions;
    std::vector<std::vector<float>> all_probabilities;

    // 对测试集每个样本进行预测并计算近似“概率”向量（投票比例）
    for (size_t t = 0; t < normalized_images_test.size(); ++t) {
        const auto& test_img = normalized_images_test[t];

        // 计算所有距离
        std::vector<std::pair<float, uint8_t>> distances;
        for (size_t i = 0; i < normalized_images_train.size(); ++i) {
            float dist = knn.compute_distance(test_img, normalized_images_train[i]);
            distances.emplace_back(dist, labels_train[i]);
        }

        // 获取前k个最近邻
        std::partial_sort(distances.begin(), distances.begin() + k, distances.end());

        std::map<uint8_t, int> vote_count;
        for (int i = 0; i < k; ++i) {
            vote_count[distances[i].second]++;
        }

        // 构造概率向量
        std::vector<float> probs(10, 0.0f);
        for (const auto& [label, count] : vote_count) {
            probs[label] = static_cast<float>(count) / k;
        }

        all_probabilities.push_back(probs);

        // 预测结果
        auto max_iter = std::max_element(probs.begin(), probs.end());
        uint8_t predicted_label = static_cast<uint8_t>(std::distance(probs.begin(), max_iter));
        predictions.push_back(predicted_label);

        if ((t + 1) % 100 == 0)
            std::cout << "Predicted " << (t + 1) << " / " << normalized_images_test.size() << "\n";
    }

    // 评估指标
    auto report = evaluate_classification(labels_test, predictions, 10);
    std::cout << "Accuracy: " << report.accuracy << "\n";
    std::cout << "Precision (macro): " << report.precision_macro << "\n";
    std::cout << "Recall (macro): " << report.recall_macro << "\n";
    std::cout << "F1 Score (macro): " << report.f1_macro << "\n";

    // 保存 CSV 文件供 Python 使用
    export_probabilities_to_csv(labels_test, all_probabilities, "knn");

    return 0;
}
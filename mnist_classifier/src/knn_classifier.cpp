// knn_classifier.cpp
#include "knn_classifier.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <utility>  // for std::pair
#include <map>

// 构造函数
KNNClassifier::KNNClassifier(int k) : k_(k) {
    if (k <= 0) {
        throw std::invalid_argument("k must be positive.");
    }
}

// 训练数据暂时只是存下来
void KNNClassifier::train(const std::vector<std::vector<float>>& images,
                         const std::vector<uint8_t>& labels) {
    if (images.size() != labels.size()) {
        throw std::runtime_error("Number of images and labels must match.");
    }
    train_images_ = images;
    train_labels_ = labels;
}

// 欧几里得距离平方
float KNNClassifier::compute_distance(const std::vector<float>& a,
                                     const std::vector<float>& b) const {
    float sum = 0.0f;
    for (size_t i = 0; i < a.size(); ++i) {
        float diff = a[i] - b[i];
        sum += diff * diff;
    }
    return sum;
}

// 预测标签
uint8_t KNNClassifier::predict(const std::vector<float>& image) const {
    if (train_images_.empty()) {
        throw std::runtime_error("Classifier has not been trained.");
    }

    // 存储 <距离, 标签>
    std::vector<std::pair<float, uint8_t>> distances;

    for (size_t i = 0; i < train_images_.size(); ++i) {
        float dist = compute_distance(image, train_images_[i]);
        distances.emplace_back(dist, train_labels_[i]);
    }

    // 按距离排序，找前k个
    std::partial_sort(distances.begin(), distances.begin() + k_, distances.end());

    // 投票计数
    std::map<uint8_t, int> vote_count;
    for (int i = 0; i < k_; ++i) {
        uint8_t label = distances[i].second;
        vote_count[label]++;
    }

    // 返回得票最多的标签
    uint8_t best_label = 0;
    int max_votes = 0;
    for (const auto& [label, count] : vote_count) {
        if (count > max_votes) {
            max_votes = count;
            best_label = label;
        }
    }

    return best_label;
}
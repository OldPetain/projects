// decision_tree.cpp
#include "decision_tree.hpp"
#include <iostream>
#include <algorithm>
#include <limits>
#include <map>

// 决策树初始化
DecisionTree::DecisionTree(int max_depth, int min_samples_split)
    : root_(nullptr), max_depth_(max_depth), min_samples_split_(min_samples_split) {}

// 决策树训练
void DecisionTree::train(const std::vector<std::vector<float>>& images,
                         const std::vector<uint8_t>& labels) {
    std::cout << "Training decision tree...\n";
    // 进入递归构建树的过程
    root_ = build_tree(images, labels, 0);
}

// 决策树预测
uint8_t DecisionTree::predict(const std::vector<float>& image) const {
    return predict_from_node(image, root_);
}

// 递归构建树
DecisionTree::Node* DecisionTree::build_tree(
    const std::vector<std::vector<float>>& images,
    const std::vector<uint8_t>& labels,
    int depth
) {
    std::cout << "Building node at depth " << depth << " with " << labels.size() << " samples.\n";
    
    // map 用于统计标签数量
    std::map<uint8_t, int> label_count;
    for (auto label : labels) label_count[label]++;

    // 如果样本数少 或 达到最大深度，创建叶节点
    if (depth >= max_depth_ || labels.size() < min_samples_split_) {
        uint8_t most_common = std::max_element(
            label_count.begin(), label_count.end(),
            [](auto a, auto b) { return a.second < b.second; })->first;
        auto* node = new Node();
        node->is_leaf = true;
        node->class_label = most_common;
        return node;
    }
    
    int best_feature = -1;// 最佳分裂特征
    float best_threshold = 0.0f; // 最佳分裂阈值
    float best_score = std::numeric_limits<float>::max(); // 最佳分裂得分
    std::vector<size_t> best_left_idx, best_right_idx; //定义左右子节点样本索引

    for (size_t f = 0; f < images[0].size(); ++f) {
        // 
        std::vector<float> values;
        for (const auto& img : images) values.push_back(img[f]);
        std::sort(values.begin(), values.end());

        for (size_t i = 1; i < values.size(); i += values.size() / 10 + 1) {
            float threshold = values[i];
            std::vector<size_t> left, right;
            for (size_t j = 0; j < images.size(); ++j) {
                if (images[j][f] < threshold) left.push_back(j);
                else right.push_back(j);
            }
            if (left.empty() || right.empty()) continue;

            std::vector<uint8_t> left_labels, right_labels;
            for (auto idx : left) left_labels.push_back(labels[idx]);
            for (auto idx : right) right_labels.push_back(labels[idx]);

            float score = compute_split_score(left_labels, right_labels);
            if (score < best_score) {
                best_score = score;
                best_feature = f;
                best_threshold = threshold;
                best_left_idx = left;
                best_right_idx = right;
            }
        }
    }

    if (best_feature == -1) {
        uint8_t most_common = std::max_element(
            label_count.begin(), label_count.end(),
            [](auto a, auto b) { return a.second < b.second; })->first;
        auto* node = new Node();
        node->is_leaf = true;
        node->class_label = most_common;
        return node;
    }

    std::vector<std::vector<float>> left_images, right_images;
    std::vector<uint8_t> left_labels, right_labels;

    for (auto idx : best_left_idx) {
        left_images.push_back(images[idx]);
        left_labels.push_back(labels[idx]);
    }
    for (auto idx : best_right_idx) {
        right_images.push_back(images[idx]);
        right_labels.push_back(labels[idx]);
    }

    auto* node = new Node();
    node->is_leaf = false;
    node->feature_index = best_feature;
    node->threshold = best_threshold;
    node->left = build_tree(left_images, left_labels, depth + 1);
    node->right = build_tree(right_images, right_labels, depth + 1);

    return node;
}

float DecisionTree::gini_impurity(const std::vector<uint8_t>& labels) const {
    std::map<uint8_t, int> count;
    for (auto l : labels) count[l]++;
    float impurity = 1.0f;
    int total = labels.size();
    for (auto& [label, cnt] : count) {
        float p = static_cast<float>(cnt) / total;
        impurity -= p * p;
    }
    return impurity;
}

float DecisionTree::compute_split_score(const std::vector<uint8_t>& left,
                                        const std::vector<uint8_t>& right) const {
    int total = left.size() + right.size();
    float score = (left.size() * gini_impurity(left) +
                   right.size() * gini_impurity(right)) / total;
    return score;
}

uint8_t DecisionTree::predict_from_node(const std::vector<float>& image, Node* node) const {
    if (node->is_leaf) return node->class_label;
    if (image[node->feature_index] < node->threshold)
        return predict_from_node(image, node->left);
    else
        return predict_from_node(image, node->right);
}
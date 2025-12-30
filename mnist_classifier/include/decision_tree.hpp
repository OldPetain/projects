//decision_tree.hpp
#pragma once
#include <vector>
#include <cstdint>

class DecisionTree {
public:
    DecisionTree(int max_depth = 10, int min_samples_split = 5);

    void train(const std::vector<std::vector<float>>& images,
               const std::vector<uint8_t>& labels);

    uint8_t predict(const std::vector<float>& image) const;

private:
    struct Node {
        bool is_leaf;
        uint8_t class_label;
        int feature_index;
        float threshold;
        Node* left;
        Node* right;

        Node() : is_leaf(true), class_label(0), feature_index(-1),
                 threshold(0.0f), left(nullptr), right(nullptr) {}
    };

    Node* build_tree(const std::vector<std::vector<float>>& images,
                     const std::vector<uint8_t>& labels,
                     int depth);

    uint8_t predict_from_node(const std::vector<float>& image, Node* node) const;

    float gini_impurity(const std::vector<uint8_t>& labels) const;
    float compute_split_score(const std::vector<uint8_t>& left,
                              const std::vector<uint8_t>& right) const;

    Node* root_;
    int max_depth_;
    int min_samples_split_;
};

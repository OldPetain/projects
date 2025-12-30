// knn_classifier.hpp
#pragma once

#include <vector>
#include <cstdint>
#include <string>

class KNNClassifier
{
public:
    KNNClassifier(int k);

    void train(const std::vector<std::vector<float>> &images,
               const std::vector<uint8_t> &labels);

    uint8_t predict(const std::vector<float> &image) const;
    float compute_distance(const std::vector<float> &a,
                           const std::vector<float> &b) const;

private:
    int k_;
    std::vector<std::vector<float>> train_images_;
    std::vector<uint8_t> train_labels_;
};
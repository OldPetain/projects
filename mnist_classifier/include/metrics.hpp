// metrics.hpp
#pragma once

#include <vector>
#include <cstdint>
#include <map>
#include <string>

// 分类评估结构体
struct ClassificationReport {
    float accuracy;
    float precision_macro;
    float recall_macro;
    float f1_macro;
    float auc_macro;

    std::map<std::string, float> per_class_precision;
    std::map<std::string, float> per_class_recall;
    std::map<std::string, float> per_class_f1;
};

// 分类评估：精度、召回率、F1、准确率
ClassificationReport evaluate_classification(
    const std::vector<uint8_t>& y_true,
    const std::vector<uint8_t>& y_pred,
    int num_classes);

// 保存预测概率和真实标签到 output/prob_output.csv
void export_probabilities_to_csv(
    const std::vector<uint8_t>& labels,
    const std::vector<std::vector<float>>& probabilities,
    const std::string& model_name = "model");

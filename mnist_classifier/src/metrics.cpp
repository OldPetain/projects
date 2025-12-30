// metrics.cpp
#include "metrics.hpp"
#include <cmath>
#include <stdexcept>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

ClassificationReport evaluate_classification(
    const std::vector<uint8_t>& y_true,
    const std::vector<uint8_t>& y_pred,
    int num_classes)
{
    if (y_true.size() != y_pred.size()) {
        throw std::runtime_error("Length mismatch between true and predicted labels");
    }

    std::vector<int> TP(num_classes, 0), FP(num_classes, 0), FN(num_classes, 0);
    int total_correct = 0;
    int total = y_true.size();

    for (size_t i = 0; i < y_true.size(); ++i) {
        int t = y_true[i];
        int p = y_pred[i];
        if (t == p) {
            total_correct++;
            TP[t]++;
        } else {
            FP[p]++;
            FN[t]++;
        }
    }

    float accuracy = static_cast<float>(total_correct) / total;

    float precision_macro = 0.0f, recall_macro = 0.0f, f1_macro = 0.0f;
    std::map<std::string, float> per_class_precision, per_class_recall, per_class_f1;

    for (int i = 0; i < num_classes; ++i) {
        float prec = TP[i] + FP[i] > 0 ? static_cast<float>(TP[i]) / (TP[i] + FP[i]) : 0.0f;
        float rec = TP[i] + FN[i] > 0 ? static_cast<float>(TP[i]) / (TP[i] + FN[i]) : 0.0f;
        float f1 = prec + rec > 0 ? 2 * prec * rec / (prec + rec) : 0.0f;

        precision_macro += prec;
        recall_macro += rec;
        f1_macro += f1;

        per_class_precision[std::to_string(i)] = prec;
        per_class_recall[std::to_string(i)] = rec;
        per_class_f1[std::to_string(i)] = f1;
    }

    precision_macro /= num_classes;
    recall_macro /= num_classes;
    f1_macro /= num_classes;

    ClassificationReport report;
    report.accuracy = accuracy;
    report.precision_macro = precision_macro;
    report.recall_macro = recall_macro;
    report.f1_macro = f1_macro;
    report.auc_macro = -1.0f;

    report.per_class_precision = per_class_precision;
    report.per_class_recall = per_class_recall;
    report.per_class_f1 = per_class_f1;

    return report;
}

void export_probabilities_to_csv(
    const std::vector<uint8_t>& labels,
    const std::vector<std::vector<float>>& probabilities,
    const std::string& model_name)
{
    if (labels.size() != probabilities.size()) {
        throw std::runtime_error("Size mismatch between labels and probabilities");
    }

    // 确保 output 目录存在
    fs::create_directories("output");

    std::string filename = "output/" + model_name + "_prob_output.csv";
    std::ofstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + filename);
    }

    int num_classes = probabilities[0].size();

    // 写表头
    file << "label";
    for (int i = 0; i < num_classes; ++i) {
        file << ",prob_" << i;
    }
    file << "\n";

    // 写每一行数据
    for (size_t i = 0; i < labels.size(); ++i) {
        file << static_cast<int>(labels[i]);
        for (float prob : probabilities[i]) {
            file << "," << prob;
        }
        file << "\n";
    }

    file.close();
    std::cout << "[信息] 已将预测概率保存至文件: " << filename << std::endl;
}
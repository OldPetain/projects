// main_SVM.cpp
#include "mnist_reader.hpp"
#include "svm_classifier.hpp"
#include "metrics.hpp"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <vector>
#include <numeric>
#include <random>

int main()
{
    const size_t TRAIN_SAMPLES_TO_USE = 3000;
    const float TARGET_ACCURACY = 0.90f;

    auto images_train_full = MNISTReader::load_images("data/train-images-idx3-ubyte");
    auto labels_train_full = MNISTReader::load_labels("data/train-labels-idx1-ubyte");
    auto images_test_full = MNISTReader::load_images("data/t10k-images-idx3-ubyte");
    auto labels_test_full = MNISTReader::load_labels("data/t10k-labels-idx1-ubyte");

    if (images_train_full.size() < TRAIN_SAMPLES_TO_USE)
    {
        std::cerr << "错误：原始训练样本数不足。\n";
        return 1;
    }

    auto normalized_test_full = MNISTReader::normalize_images(images_test_full);
    std::random_device rd;
    std::mt19937 g(rd());

    int cnt = 0;

    while (true)
    {
        SVMClassifier svm;
        //svm.set_params(1.0, "rbf", true); // <--- 启用概率输出

        std::vector<int> indices(images_train_full.size());
        std::iota(indices.begin(), indices.end(), 0);
        std::shuffle(indices.begin(), indices.end(), g);

        std::vector<std::vector<uint8_t>> images_train_sampled;
        std::vector<uint8_t> labels_train_sampled;
        images_train_sampled.reserve(TRAIN_SAMPLES_TO_USE);
        labels_train_sampled.reserve(TRAIN_SAMPLES_TO_USE);

        for (size_t i = 0; i < TRAIN_SAMPLES_TO_USE; ++i)
        {
            images_train_sampled.push_back(images_train_full[indices[i]]);
            labels_train_sampled.push_back(labels_train_full[indices[i]]);
        }

        auto normalized_train_sampled = MNISTReader::normalize_images(images_train_sampled);

        std::cout << "\n[训练] 第 " << cnt + 1 << " 个模型...\n";
        svm.train(normalized_train_sampled, labels_train_sampled);

        std::vector<uint8_t> predictions;
        std::vector<std::vector<float>> probabilities;

        try
        {
            for (const auto &img : normalized_test_full)
            {
                auto prob = svm.predict_proba(img);

                if (prob.empty())
                    throw std::runtime_error("predict_proba 返回空向量，请确认是否启用概率预测。");

                auto pred = static_cast<uint8_t>(
                    std::distance(prob.begin(), std::max_element(prob.begin(), prob.end())));
                predictions.push_back(pred);
                probabilities.push_back(prob);
            }
        }
        catch (const std::exception &e)
        {
            std::cerr << "[警告] 概率预测失败：" << e.what() << "\n";
            std::cerr << "       使用 fallback: predict()\n";

            predictions.clear();
            probabilities.clear();

            for (const auto &img : normalized_test_full)
                predictions.push_back(svm.predict(img));
        }

        auto report = evaluate_classification(labels_test_full, predictions, 10);

        std::cout << std::fixed << std::setprecision(4);
        std::cout << "[评估] 准确率: " << report.accuracy << "\n";
        std::cout << "       精确率 (macro): " << report.precision_macro << "\n";
        std::cout << "       召回率 (macro): " << report.recall_macro << "\n";
        std::cout << "       F1 分数 (macro): " << report.f1_macro << "\n";

        // 组装文件名（带编号和准确率）
        std::ostringstream oss;
        oss << "model/mnist_svm_" << cnt
            << "_acc_" << std::setprecision(3) << report.accuracy << ".model";
        std::string model_path = oss.str();

        svm.save_model(model_path);

        if (!probabilities.empty())
        {
            std::ostringstream csvname;
            csvname << "svm_" << cnt
                    << "_acc_" << std::setprecision(3) << report.accuracy;

            export_probabilities_to_csv(labels_test_full, probabilities, csvname.str());
        }

        if (report.accuracy >= TARGET_ACCURACY)
        {
            std::cout << "\n已达到目标准确率 " << TARGET_ACCURACY << "，训练停止。\n";
            break;
        }

        cnt++;
    }

    return 0;
}
//svm_classifier.hpp
#pragma once
#include <vector>
#include <cstdint>
#include <string>
#include <svm.h> // LIBSVM 库头文件

class SVMClassifier {
public:
    // 构造函数：初始化 SVM 参数
    SVMClassifier();
    // 析构函数：释放模型内存
    ~SVMClassifier();

    // 训练模型
    void train(const std::vector<std::vector<float>>& images,
               const std::vector<uint8_t>& labels);

    // 预测单个图像的类别
    uint8_t predict(const std::vector<float>& image) const;
    // 预测单个图像的类别概率
    std::vector<float> predict_proba(const std::vector<float>& x) const;

    // 从文件加载模型
    bool load_model(const std::string& path);
    // 将模型保存到文件
    void save_model(const std::string& path) const;

    // set_params 函数已移除

private:
    struct svm_model* model_; // 存储训练好的 SVM 模型
    struct svm_parameter param_; // 存储 SVM 训练参数
    int num_classes_; // 分类类别数量（例如 MNIST 是 10）
};
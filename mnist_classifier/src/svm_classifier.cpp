// svm_classifier.cpp
#include "svm_classifier.hpp"
#include <stdexcept>
#include <iostream>
#include <filesystem> // 用于文件系统操作，如检查文件是否存在，创建目录
#include <regex>      // 用于解析 LIBSVM 输出

namespace fs = std::filesystem; // 简化 std::filesystem 的使用

// LIBSVM 库的内部打印回调函数
// 简化此函数，只打印关键的优化完成和总支持向量数信息
void chinese_info(const char* s) {
    std::string str(s);
    if (str.find("optimization finished") != std::string::npos) {
        std::cout << "优化完成\n";
    } else if (str.find("Total nSV") != std::string::npos) {
        std::regex total_sv_regex(R"(Total nSV\s*=\s*(\d+))");
        std::smatch match;
        if (std::regex_search(str, match, total_sv_regex)) {
            std::cout << "总支持向量数：" << match[1] << "\n";
        }
    } else {
        // 对于其他 LIBSVM 输出，直接打印原文，或者完全省略
        // std::cout << str; // 如果想完全静默，可以注释掉这行
    }
}

// LIBSVM 库要求的回调函数签名
static void print_chinese_info(const char* s) {
    chinese_info(s);
}

// SVMClassifier 构造函数
SVMClassifier::SVMClassifier() : model_(nullptr), num_classes_(10) {
    // 设置 SVM 参数
    param_.svm_type = C_SVC;        // C-支持向量分类器
    param_.kernel_type = RBF;       // 径向基函数 (RBF) 核
    param_.degree = 3;              // 多项式核的度数 (RBF核中不使用)
    param_.coef0 = 0;               // 多项式/sigmoid 核的独立项 (RBF核中不使用)
    param_.nu = 0.5;                // Nu-SVC, Nu-SVR, One-class SVM 的参数 (这里是 C-SVC, 不使用)
    param_.cache_size = 100;        // 核缓存大小（MB）
    param_.eps = 1e-3;              // 停止标准容忍度
    param_.p = 0.1;                 // SVR 的损失函数参数 (这里是分类，不使用)
    param_.nr_weight = 0;           // 类别权重数量
    param_.weight_label = nullptr;  // 类别权重标签数组
    param_.weight = nullptr;        // 类别权重值数组

    // 根据您的要求，硬编码 C 和 gamma 值，以及概率和收缩标志
    param_.C = 100.0;             // 惩罚系数 C
    param_.gamma = 0.01;          // RBF 核参数 gamma
    param_.probability = 1;       // 开启概率估计
    param_.shrinking = 1;         // 开启收缩启发式
}

// SVMClassifier 析构函数
SVMClassifier::~SVMClassifier() {
    if (model_) {
        svm_free_and_destroy_model(&model_); // 释放模型内存
        model_ = nullptr; // 防止悬空指针
    }
}

// 训练 SVM 模型
void SVMClassifier::train(const std::vector<std::vector<float>>& images,
                           const std::vector<uint8_t>& labels) {
    int num_samples = images.size();
    if (num_samples == 0) {
        std::cerr << "错误：训练数据为空！\n";
        return;
    }
    if (labels.size() != num_samples) {
        std::cerr << "错误：标签数量与图像数量不一致！\n";
        return;
    }
    int dim = images[0].size(); // 特征维度
     // 检查所有图像维度是否一致
    for (size_t i = 0; i < images.size(); ++i) {
        if (images[i].size() != dim) {
            std::cerr << "错误：第 " << i << " 张图像的维度不一致！应为 " << dim
                      << "，但实际为 " << images[i].size() << "\n";
            return;
        }
    }

    struct svm_problem prob; // LIBSVM 训练问题结构体
    prob.l = num_samples;    // 样本数量
    prob.y = new double[num_samples]; // 标签数组
    prob.x = new struct svm_node*[num_samples]; // 特征节点数组

    // 填充 svm_problem 结构体
    for (int i = 0; i < num_samples; ++i) {
        prob.y[i] = labels[i]; // 设置标签
        prob.x[i] = new svm_node[dim + 1]; // +1 是为了LIBSVM要求以-1结束的节点
        for (int j = 0; j < dim; ++j) {
            prob.x[i][j].index = j + 1; // LIBSVM 特征索引从 1 开始
            prob.x[i][j].value = images[i][j]; // 特征值
        }
        prob.x[i][dim].index = -1; // 标记节点数组结束
    }

    // 检查 SVM 参数
    const char* error_msg = svm_check_parameter(&prob, &param_);
    if (error_msg) {
        // 参数检查失败，释放内存并抛出异常
        for (int i = 0; i < num_samples; ++i) {
            delete[] prob.x[i];
        }
        delete[] prob.x;
        delete[] prob.y;
        throw std::runtime_error(error_msg);
    }

    std::cout << "开始训练支持向量机模型...\n";
    // 设置 LIBSVM 的打印回调函数，以便输出中文信息
    svm_set_print_string_function(print_chinese_info);

    // 如果模型已存在，先释放旧模型，确保每次训练都得到新模型
    if (model_) {
        svm_free_and_destroy_model(&model_);
        model_ = nullptr;
    }

    // 调用 LIBSVM 训练函数
    model_ = svm_train(&prob, &param_);

    std::cout << "训练完成。\n";

    // 释放为 svm_problem 分配的内存
    for (int i = 0; i < num_samples; ++i) {
        delete[] prob.x[i];
    }
    delete[] prob.x;
    delete[] prob.y;
}

// 预测单个图像的类别
uint8_t SVMClassifier::predict(const std::vector<float>& image) const {
    if (!model_) {
        throw std::runtime_error("模型未加载或未训练。无法进行预测。");
    }
    int dim = image.size();
    svm_node* x = new svm_node[dim + 1]; // 特征节点数组
    for (int i = 0; i < dim; ++i) {
        x[i].index = i + 1;
        x[i].value = image[i];
    }
    x[dim].index = -1; // 标记数组结束

    double label = svm_predict(model_, x); // 进行预测
    delete[] x; // 释放内存
    return static_cast<uint8_t>(label); // 返回预测类别
}

std::vector<float> SVMClassifier::predict_proba(const std::vector<float>& image) const {
    if (!model_ || param_.probability != 1) {
        throw std::runtime_error("模型未加载，或未开启概率估计。");
    }
    int dim = image.size();
    svm_node* x = new svm_node[dim + 1];
    for (int i = 0; i < dim; ++i) {
        x[i].index = i + 1;
        x[i].value = image[i];
    }
    x[dim].index = -1;

    int nr_class = svm_get_nr_class(model_);
    int* labels = new int[nr_class];
    svm_get_labels(model_, labels); // 获取类别顺序

    double* prob_estimates = new double[nr_class];
    svm_predict_probability(model_, x, prob_estimates);

    // 初始化结果概率，默认0
    std::vector<float> probs(num_classes_, 0.0f);

    // 按类别标签对应的索引赋值概率
    for (int i = 0; i < nr_class; ++i) {
        int label = labels[i]; // LIBSVM类别标签
        if (label >= 0 && label < num_classes_) {
            probs[label] = static_cast<float>(prob_estimates[i]);
        } else {
            // 如果标签超范围，可打印警告或忽略
            std::cerr << "警告：类别标签 " << label << " 超出范围 0-" << num_classes_ - 1 << "\n";
        }
    }

    delete[] labels;
    delete[] prob_estimates;
    delete[] x;
    return probs;
}


// 从文件加载模型
bool SVMClassifier::load_model(const std::string& path) {
    if (!fs::exists(path)) {
        return false; // 文件不存在
    }
    if (model_) {
        svm_free_and_destroy_model(&model_); // 释放旧模型
        model_ = nullptr;
    }
    model_ = svm_load_model(path.c_str()); // 加载模型
    if (model_) {
        num_classes_ = svm_get_nr_class(model_); // 更新类别数量
        // 根据加载的模型判断是否支持概率预测
        if (svm_check_probability_model(model_) == 1) {
             param_.probability = 1;
        } else {
             param_.probability = 0;
        }
    } else {
        std::cerr << "模型加载失败！\n";
    }
    return model_ != nullptr;
}

// 将模型保存到文件
void SVMClassifier::save_model(const std::string& path) const {
    if (!model_) {
        std::cerr << "错误：模型未训练，无法保存！" << std::endl;
        return;
    }
    // 确保保存路径的目录存在
    fs::create_directories(fs::path(path).parent_path());
    if (svm_save_model(path.c_str(), model_) == 0) {
        std::cout << "模型已保存至：" << path << std::endl;
    } else {
        std::cerr << "模型保存失败！" << std::endl;
    }
}
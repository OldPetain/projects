一、重新编译应该执行哪些命令？
1.如果当前在目录 /home/dhys/SDL/mnist_classifier/ 下，则执行以下命令：
(1)只是修改了源代码：
cd build
make
(2)如果你修改了 CMakeLists.txt 或新增源文件：
cd build
cmake ..
make

二、如何执行测试？
1.在 build 目录下，执行以下命令：
cd ..                     # 回到 mnist_classifier 根目录(为了源文件里的相对路径能够正确取得数据集)
./build/test_mnist        # 执行测试程序

三、如何选用主函数？
1.从main文件夹中选择一个主函数，如main_svm.cpp，将其复制。
2.找到test文件夹中的main.cpp，全选粘贴覆盖原文件内容。

四、如何获得各种方法的准确率等原始数据的比较表格和如何绘制ROC曲线？
1.在根目录下，执行
source venv/bin/activate
2.再执行
python3 draw_metrics.py

附录：1.svm错误原因
// 预测单个图像的类别概率
std::vector<float> SVMClassifier::predict_proba(const std::vector<float>& image) const {
    if (!model_ || param_.probability != 1) {
        throw std::runtime_error("模型未加载，或未开启概率估计 (param_.probability != 1)。无法进行概率预测。");
    }
    int dim = image.size();
    svm_node* x = new svm_node[dim + 1]; // 特征节点数组
    for (int i = 0; i < dim; ++i) {
        x[i].index = i + 1;
        x[i].value = image[i];
    }
    x[dim].index = -1; // 标记数组结束

    std::vector<float> probs(num_classes_, 0.0f);
    double* prob_estimates = new double[num_classes_]; // 存储概率估计

    // 进行概率预测
    svm_predict_probability(model_, x, prob_estimates);

    // 将 double 类型的概率复制到 float 类型的 vector 中
    for (int i = 0; i < num_classes_; ++i) {
        probs[i] = static_cast<float>(prob_estimates[i]);
    }

    delete[] x; // 释放内存
    delete[] prob_estimates; // 释放内存
    return probs; // 返回概率向量
}
输出的概率并不是按0、1、2、3...顺序排列的，而是按向量机的类别顺序排列的。
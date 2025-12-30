// main_decision_tree.cpp
#include "mnist_reader.hpp"
#include "decision_tree.hpp"
#include "metrics.hpp"
#include <iostream>

int main() {
    auto images_train = MNISTReader::load_images("data/train-images-idx3-ubyte");
    auto labels_train = MNISTReader::load_labels("data/train-labels-idx1-ubyte");
    auto images_test = MNISTReader::load_images("data/t10k-images-idx3-ubyte");
    auto labels_test = MNISTReader::load_labels("data/t10k-labels-idx1-ubyte");

    auto normalized_train = MNISTReader::normalize_images(images_train);
    auto normalized_test = MNISTReader::normalize_images(images_test);

    DecisionTree tree(10, 5);
    tree.train(normalized_train, labels_train);

    std::vector<uint8_t> predictions;
    std::vector<std::vector<float>> prob_vectors;

    for (const auto& img : normalized_test) {
        uint8_t pred = tree.predict(img);
        predictions.push_back(pred);

        std::vector<float> prob(10, 0.0f);
        prob[pred] = 1.0f;
        prob_vectors.push_back(prob);
    }

    auto report = evaluate_classification(labels_test, predictions, 10);
    std::cout << "Accuracy: " << report.accuracy << "\n";
    std::cout << "Precision (macro): " << report.precision_macro << "\n";
    std::cout << "Recall (macro): " << report.recall_macro << "\n";
    std::cout << "F1 Score (macro): " << report.f1_macro << "\n";

    export_probabilities_to_csv(labels_test, prob_vectors, "decision_tree");

    return 0;
}
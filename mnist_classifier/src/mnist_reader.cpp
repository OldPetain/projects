// mnist_reader.cpp
#include "mnist_reader.hpp"
#include <fstream>
#include <stdexcept>
#include <iostream>
#include <vector>

namespace MNISTReader {

    int32_t read_int32(std::ifstream& stream) {
        uint8_t bytes[4];
        stream.read(reinterpret_cast<char*>(bytes), 4);
        return (bytes[0] << 24) | (bytes[1] << 16) | (bytes[2] << 8) | bytes[3];
    }

    std::vector<std::vector<uint8_t>> load_images(const std::string& filename) {
        std::ifstream file(filename, std::ios::binary); // 打开二进制文件
        if (!file.is_open()) {
            throw std::runtime_error("Could not open file: " + filename);
        } // 若文件打开失败，则抛出异常
        
        // 读取魔数和图像数量
        int32_t magic_number = read_int32(file); // 读取魔数
        if (magic_number != 2051) {
            throw std::runtime_error("Invalid magic number for images: " + std::to_string(magic_number));
        } // 若魔数不正确，则抛出异常

        int32_t num_images = read_int32(file); // 读取图像数量
        int32_t num_rows = read_int32(file); // 读取图像高度
        int32_t num_cols = read_int32(file); // 读取图像宽度

        std::vector<std::vector<uint8_t>> images(num_images, std::vector<uint8_t>(num_rows * num_cols)); // 建立图像矩阵
        for (int i = 0; i < num_images; ++i) {
            file.read(reinterpret_cast<char*>(images[i].data()), num_rows * num_cols);
        }

        return images;
    }

    std::vector<uint8_t> load_labels(const std::string& filename) {
        std::ifstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            throw std::runtime_error("Could not open file: " + filename);
        }

        int32_t magic_number = read_int32(file);
        if (magic_number != 2049) {
            throw std::runtime_error("Invalid magic number for labels: " + std::to_string(magic_number));
        }

        int32_t num_labels = read_int32(file);
        std::vector<uint8_t> labels(num_labels);
        file.read(reinterpret_cast<char*>(labels.data()), num_labels);

        return labels;
    }

    std::vector<std::vector<float>> normalize_images(const std::vector<std::vector<uint8_t>>& images) {
        std::vector<std::vector<float>> normalized_images;
        normalized_images.reserve(images.size());
        for (const auto& image : images) {
            std::vector<float> normalized_image(image.size());
            for (size_t i = 0; i < image.size(); ++i) {
                normalized_image[i] = static_cast<float>(image[i]) / 255.0f;
            }
            normalized_images.push_back(normalized_image);
        }
        return normalized_images;
    }

    std::pair<std::vector<std::vector<float>>, std::vector<uint8_t>> read_training_data(const std::string& folder) {
        auto images = load_images(folder + "/train-images-idx3-ubyte");
        auto labels = load_labels(folder + "/train-labels-idx1-ubyte");
        auto normalized = normalize_images(images);
        return {normalized, labels};
    }

    std::pair<std::vector<std::vector<float>>, std::vector<uint8_t>> read_test_data(const std::string& folder) {
        auto images = load_images(folder + "/t10k-images-idx3-ubyte");
        auto labels = load_labels(folder + "/t10k-labels-idx1-ubyte");
        auto normalized = normalize_images(images);
        return {normalized, labels};
    }
}

void show_image(const std::vector<uint8_t>& image, int width, int height) {
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            std::cout << (image[i * width + j] > 128 ? '#' : ' ');
        }
        std::cout << std::endl;
    }
}

void show_image(const std::vector<float>& image, int width, int height) {
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            std::cout << (image[i * width + j] > 0.5f ? '#' : ' ');
        }
        std::cout << std::endl;
    }
}
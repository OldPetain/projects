// mnist_reader.hpp
#pragma once

#include <vector>
#include <string>
#include <cstdint>

namespace MNISTReader {
    std::vector<std::vector<uint8_t>> load_images(const std::string& filename);
    std::vector<uint8_t> load_labels(const std::string& filename);
    std::vector<std::vector<float>> normalize_images(const std::vector<std::vector<uint8_t>>& images);

    std::pair<std::vector<std::vector<float>>, std::vector<uint8_t>> read_training_data(const std::string& folder);
    std::pair<std::vector<std::vector<float>>, std::vector<uint8_t>> read_test_data(const std::string& folder);
}

void show_image(const std::vector<uint8_t>& image, int width, int height);
void show_image(const std::vector<float>& image, int width, int height);
// loss.hpp
#pragma once

#include <vector>

namespace Loss {

float cross_entropy(const std::vector<float>& prediction, const std::vector<float>& target);
std::vector<float> cross_entropy_derivative(const std::vector<float>& prediction, const std::vector<float>& target);

}
// activation.hpp
#pragma once

#include <vector>
#include <cmath>

namespace Activation {

float sigmoid(float x);
float sigmoid_derivative(float x);

float relu(float x);
float relu_derivative(float x);

std::vector<float> apply_sigmoid(const std::vector<float>& input);
std::vector<float> apply_sigmoid_deriv(const std::vector<float>& input);

std::vector<float> apply_relu(const std::vector<float>& input);
std::vector<float> apply_relu_deriv(const std::vector<float>& input);

}
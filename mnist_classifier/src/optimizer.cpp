// optimizer.cpp
#include "optimizer.hpp"
#include <iostream>

void SGD::update(std::vector<float> &weights, const std::vector<float> &grads, float lr)
{
    for (size_t i = 0; i < weights.size(); ++i)
    {
        weights[i] -= lr * grads[i];
    }
}

Momentum::Momentum(float momentum) : momentum_(momentum) {}

void Momentum::update(std::vector<float> &weights, const std::vector<float> &grads, float lr)
{
    auto it = velocity_map_.find(&weights);
    if (it == velocity_map_.end())
    {
        std::cout << "[Momentum] Reinitializing velocity. size = " << weights.size() << std::endl;
        velocity_map_[&weights] = std::vector<float>(weights.size(), 0.0f);
    }

    std::vector<float> &velocity = velocity_map_[&weights];

    for (size_t i = 0; i < weights.size(); ++i)
    {
        velocity[i] = momentum_ * velocity[i] - lr * grads[i];
        weights[i] += velocity[i];
    }
}
// optimizer.hpp
#pragma once

#include <vector>
#include <map>

class Optimizer {
public:
    virtual ~Optimizer() = default;
    virtual void update(std::vector<float>& weights, const std::vector<float>& grads, float lr) = 0;
};

class SGD : public Optimizer {
public:
    void update(std::vector<float>& weights, const std::vector<float>& grads, float lr) override;
};

class Momentum : public Optimizer {
public:
    Momentum(float momentum = 0.9f);
    void update(std::vector<float>& weights, const std::vector<float>& grads, float lr) override;

private:
    float momentum_;
    std::map<const std::vector<float>*, std::vector<float>> velocity_map_;
};
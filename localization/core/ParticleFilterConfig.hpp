#pragma once

#include <stdexcept>
#include <string>

struct ParticleFilterConfig {
    double radius = 500.0;
    float epsilon = 0.1f;
    int particleCount = 200;
    float quantile = 0.99f;
    float kld_error = 0.5f;
    int binSize = 5;
    bool use_gaussian = true;

    void validate() const {
        if (radius <= 0.0)
            throw std::invalid_argument("radius must be positive, got " + std::to_string(radius));
        if (epsilon <= 0.0f || epsilon >= 1.0f)
            throw std::invalid_argument("epsilon must be in (0, 1), got " + std::to_string(epsilon));
        if (particleCount <= 0)
            throw std::invalid_argument("particleCount must be positive, got " + std::to_string(particleCount));
        if (quantile <= 0.0f || quantile > 1.0f)
            throw std::invalid_argument("quantile must be in (0, 1], got " + std::to_string(quantile));
        if (kld_error <= 0.0f)
            throw std::invalid_argument("kld_error must be positive, got " + std::to_string(kld_error));
        if (binSize <= 0)
            throw std::invalid_argument("binSize must be positive, got " + std::to_string(binSize));
    }
};

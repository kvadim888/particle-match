#pragma once

struct ParticleFilterConfig {
    double radius = 500.0;
    float epsilon = 0.1f;
    int particleCount = 200;
    float quantile = 0.99f;
    float kld_error = 0.5f;
    int binSize = 5;
    bool use_gaussian = true;
};

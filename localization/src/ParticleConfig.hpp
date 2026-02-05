//
// Shared configuration for all particles in a filter instance.
// Replaces the former static mutable state in Particle.
//

#pragma once

#include <vector>
#include <opencv2/core/types.hpp>

struct ParticleConfig {
    double direction = 0.0;
    cv::Point2i mapCenter;
    std::vector<float> r_initial;
    float r_step = 0.05f;

    ParticleConfig() {
        // -5deg ~ +5deg rotation range
        r_initial = {
            -(3 * r_step),
            -(2.5f * r_step),
            -(2 * r_step),
            -(1.5f * r_step),
            -(r_step),
            -(0.5f * r_step),
            0.f,
            0.5f * r_step,
            r_step,
            1.5f * r_step,
            2 * r_step,
            2.5f * r_step,
            3 * r_step
        };
    }

    void setMapDimensions(const cv::Size& dims) {
        mapCenter.y = dims.height / 2;
        mapCenter.x = dims.width / 2;
    }
};

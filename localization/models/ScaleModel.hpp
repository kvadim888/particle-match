#pragma once

#include <cstdint>
#include <functional>

class ScaleModel {
public:
    static constexpr float kScaleMarginLow = 0.9f;
    static constexpr float kScaleMarginHigh = 1.1f;

    float updateScale(float hfov,
                      float altitude,
                      uint32_t imageWidth,
                      const std::function<void(float, float)> &setScale) const;
};

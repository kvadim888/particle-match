#pragma once

#include <cstdint>
#include <functional>

class ScaleModel {
public:
    float updateScale(float hfov,
                      float altitude,
                      uint32_t imageWidth,
                      const std::function<void(float, float)> &setScale) const;
};

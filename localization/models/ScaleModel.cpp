#include "models/ScaleModel.hpp"

#include <cmath>

float ScaleModel::updateScale(float hfov,
                              float altitude,
                              uint32_t imageWidth,
                              const std::function<void(float, float)> &setScale) const {
    float currentScale = (std::tan(hfov / 2.0f) * altitude) / (static_cast<float>(imageWidth) / 2.0f);
    setScale(currentScale * 0.9f, currentScale * 1.1f);
    return currentScale;
}

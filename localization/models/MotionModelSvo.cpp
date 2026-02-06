#include "models/MotionModelSvo.hpp"

#include <cmath>

SvoMovementResult MotionModelSvo::getMovementFromSvo(const MetadataEntry &metadata,
                                                     const std::shared_ptr<GeographicLib::LocalCartesian> &svoCoordinates,
                                                     double direction,
                                                     const cv::Point &svoCurPosition) const {
    double lat, lon, h;
    // I had to negate both X and Y to achieve good combination
    svoCoordinates->Reverse(
            metadata.svoPose.getX(),
            metadata.svoPose.getY(),
            metadata.svoPose.getZ(),
            lat,
            lon,
            h
    );
    cv::Point curLoc = metadata.mapper->toPixels(lat, lon);
    cv::Point movement = curLoc - svoCurPosition;

    // Don't use direction from SVO, it may be misleading, just use the distance from odometry
    // and direction from compass which is way more reliable.
    float distance = static_cast<float>(std::sqrt(std::pow(movement.x, 2.f) + std::pow(movement.y, 2.f)));
    movement = cv::Point2f(
            static_cast<float>(std::sin(direction) * distance),
            static_cast<float>(-std::cos(direction) * distance)
    );

    return {movement, curLoc};
}

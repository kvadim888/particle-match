#pragma once

#include <memory>

#include <GeographicLib/LocalCartesian.hpp>
#include <fastmatch-dataset/MetadataEntry.hpp>
#include <opencv2/core/types.hpp>

struct SvoMovementResult {
    cv::Point movement;
    cv::Point updatedPosition;
};

class MotionModelSvo {
public:
    SvoMovementResult getMovementFromSvo(const MetadataEntry &metadata,
                                         const std::shared_ptr<GeographicLib::LocalCartesian> &svoCoordinates,
                                         double direction,
                                         const cv::Point &svoCurPosition) const;
};

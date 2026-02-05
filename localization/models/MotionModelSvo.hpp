#pragma once

#include <memory>

#include <GeographicLib/LocalCartesian.hpp>
#include <fastmatch-dataset/MetadataEntry.hpp>
#include <opencv2/core/types.hpp>

class MotionModelSvo {
public:
    cv::Point getMovementFromSvo(const MetadataEntry &metadata,
                                 const std::shared_ptr<GeographicLib::LocalCartesian> &svoCoordinates,
                                 double direction,
                                 cv::Point &svoCurPosition) const;

    cv::Point getMovementFromSvo2(const MetadataEntry &metadata,
                                  const std::shared_ptr<GeographicLib::LocalCartesian> &svoCoordinates,
                                  double direction,
                                  cv::Point &svoCurPosition) const;
};

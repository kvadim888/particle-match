#pragma once

#include <cstdint>
#include <iosfwd>

#include <opencv2/core/types.hpp>

class ResultWriter {
public:
    static void appendHeader(std::ostream &out);
    static void appendRow(std::ostream &out,
                          uint32_t particleCount,
                          const cv::Point &relativeLocation,
                          double distance,
                          double svoDistance);
};

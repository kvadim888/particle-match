//
// Common geometry utility functions
//

#pragma once

#include <opencv2/core/types.hpp>

/**
 * Check if a point is within the rectangle defined by top_left and bottom_right.
 * Replaces the old WITHIN macro with a type-safe inline function.
 */
template<typename PointT, typename BoundT>
inline bool isWithinBounds(const PointT& val, const BoundT& top_left, const BoundT& bottom_right) {
    return val.x > top_left.x && val.y > top_left.y &&
           val.x < bottom_right.x && val.y < bottom_right.y;
}

// Named constants that were previously magic numbers
namespace geometry {
    constexpr double kBoundaryPadding = 10.0;
    constexpr double kRadToDeg = 57.2957795130823;  // 180 / M_PI
}

#include "models/MotionModelSvo.hpp"

#include <cmath>

#include <Eigen/Eigen>
#include <opencv2/core/eigen.hpp>

#include "src/Utilities.hpp"

cv::Point MotionModelSvo::getMovementFromSvo(const MetadataEntry &metadata,
                                             const std::shared_ptr<GeographicLib::LocalCartesian> &svoCoordinates,
                                             double direction,
                                             cv::Point &svoCurPosition) const {
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

    svoCurPosition = curLoc;
    return movement;
}

cv::Point MotionModelSvo::getMovementFromSvo2(const MetadataEntry &metadata,
                                              const std::shared_ptr<GeographicLib::LocalCartesian> &svoCoordinates,
                                              double direction,
                                              cv::Point &svoCurPosition) const {
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
    cv::Mat cameraRot = Utilities::eulerAnglesToRotationMatrix(cv::Point3d(-M_PI_2, 0, 0));
    cv::Mat zeroLookVector = (cv::Mat_<double>(3, 1) << 0.0, 1.0, 0.0);
    Eigen::Quaterniond q(
            metadata.imuOrientation.getW(),
            metadata.imuOrientation.getX(),
            metadata.imuOrientation.getY(),
            metadata.imuOrientation.getZ()
            );
    q.normalize();
    cv::Mat quatTransform(3, 3, CV_64FC1); //
    cv::eigen2cv(q.toRotationMatrix(), quatTransform);
    cv::Mat cameraLookVec = (quatTransform * cameraRot) * zeroLookVector;
    // Z is used from barometer data
    cv::Point3d planePos = cv::Point3d(curLoc.x, curLoc.y, metadata.altitude);
    cv::Point3d lookVector = cv::Point3d(
            planePos.x + cameraLookVec.at<double>(0, 0),
            planePos.y + cameraLookVec.at<double>(1, 0),
            planePos.z + cameraLookVec.at<double>(2, 0)
    );
    cv::Point3d isection = Utilities::intersectPlaneV3(planePos, lookVector, cv::Point3d(0, 0, 0), cv::Point3d(0, 0, 1));
    curLoc = cv::Point(static_cast<int>(isection.x), static_cast<int>(isection.y));
    cv::Point movement = curLoc - svoCurPosition;

    // Don't use direction from SVO, it may be misleading, just use the distance from odometry
    // and direction from compass which is way more reliable.
    float distance = static_cast<float>(std::sqrt(std::pow(movement.x, 2.f) + std::pow(movement.y, 2.f)));
    movement = cv::Point2f(
            static_cast<float>(std::sin(direction) * distance),
            static_cast<float>(-std::cos(direction) * distance)
    );

    svoCurPosition = curLoc;
    return movement;
}

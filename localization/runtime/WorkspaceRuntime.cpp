#include "WorkspaceRuntime.hpp"

#include <cmath>
#include <iostream>

#include <Eigen/Eigen>
#include <opencv2/core/eigen.hpp>

#include <src/Utilities.hpp>

void WorkspaceRuntime::initialize(const MetadataEntry &metadata, const ParticleFilterConfig &config) {
    std::cout << "Initializing...";
    std::cout.flush();
    direction = metadata.imuOrientation.toRPY().getZ();
    svoCurPosition = metadata.mapLocation;
    svoCoordinates = std::make_shared<GeographicLib::LocalCartesian>(
            metadata.latitude,
            metadata.longitude,
            metadata.altitude
    );
    core.initialize(metadata, config);
    core.setDirection(direction);
    map = metadata.map;
    cv::Mat templ = metadata.getImageColored();
    updateScale(1.0f, static_cast<float>(metadata.altitude), templ.cols);
    startLocation = core.getFilter()->getPredictedLocation();
    std::cout << " done!" << std::endl;
}

void WorkspaceRuntime::update(const MetadataEntry &metadata) {
    cv::Point movement = getMovementFromSvo(metadata);
    updateScale(1.0f, static_cast<float>(metadata.altitude), 640);
    direction = metadata.imuOrientation.toRPY().getZ();
    core.setDirection(direction);
    cv::Mat templ = metadata.getImageColored();
    core.setTemplate(templ);
    if(!affineMatching) {
        corners = core.filterParticles(movement, bestTransform);
        bestView = core.getBestParticleView(metadata.map);
    } else {
#ifdef USE_CV_GPU
        corners = core.filterParticlesAffine(movement, bestTransform);
#else
        throw std::runtime_error("Affine particle matching is available with GPU support only");
#endif
    }
}

bool WorkspaceRuntime::preview(const MetadataEntry &metadata, const cv::Mat &image, std::stringstream &stringOutput) {
    return renderer.render(
            metadata,
            image,
            stringOutput,
            map,
            startLocation,
            svoCurPosition,
            direction,
            corners,
            bestTransform,
            bestView,
            core.getFilter()
    );
}

bool WorkspaceRuntime::isAffineMatching() const {
    return affineMatching;
}

void WorkspaceRuntime::setAffineMatching(bool affineMatching) {
    WorkspaceRuntime::affineMatching = affineMatching;
}

bool WorkspaceRuntime::isDisplayImage() const {
    return renderer.isDisplayImage();
}

void WorkspaceRuntime::setDisplayImage(bool displayImage) {
    renderer.setDisplayImage(displayImage);
}

void WorkspaceRuntime::setWriteImageToDisk(bool writeImageToDisk) {
    renderer.setWriteImageToDisk(writeImageToDisk);
}

void WorkspaceRuntime::setOutputDirectory(const std::string &outputDirectory) {
    renderer.setOutputDirectory(outputDirectory);
}

void WorkspaceRuntime::setCorrelationLowBound(float bound) {
    core.setLowBound(bound);
}

void WorkspaceRuntime::setConversionMethod(ParticleFastMatch::ConversionMode method) {
    core.setConversionMethod(method);
}

void WorkspaceRuntime::describe() const {
    core.describe();
}

const Particles &WorkspaceRuntime::getParticles() const {
    return core.getParticles();
}

void WorkspaceRuntime::updateScale(float hfov, float altitude, uint32_t imageWidth) {
    currentScale = (tan(hfov / 2.0f) * altitude) / (static_cast<float>(imageWidth) / 2.0f);
    core.setScale(currentScale * .9f, currentScale * 1.1f);
}

cv::Point WorkspaceRuntime::getMovementFromSvo(const MetadataEntry &metadata) {
    double lat, lon, h;
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

    float distance = static_cast<float>(std::sqrt(std::pow(movement.x, 2.f) + std::pow(movement.y, 2.f)));
    movement = cv::Point2f(
            static_cast<float>(std::sin(direction) * distance),
            static_cast<float>(-std::cos(direction) * distance)
    );

    svoCurPosition = curLoc;
    return movement;
}

cv::Point WorkspaceRuntime::getMovementFromSvo2(const MetadataEntry &metadata) {
    double lat, lon, h;
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
    cv::Mat quatTransform(3, 3, CV_64FC1);
    cv::eigen2cv(q.toRotationMatrix(), quatTransform);
    cv::Mat cameraLookVec = (quatTransform * cameraRot) * zeroLookVector;
    cv::Point3d planePos = cv::Point3d(curLoc.x, curLoc.y, metadata.altitude);
    cv::Point3d lookVector = cv::Point3d(
            planePos.x + cameraLookVec.at<double>(0, 0),
            planePos.y + cameraLookVec.at<double>(1, 0),
            planePos.z + cameraLookVec.at<double>(2, 0)
    );
    cv::Point3d isection = Utilities::intersectPlaneV3(planePos, lookVector, cv::Point3d(0, 0, 0), cv::Point3d(0, 0, 1));
    curLoc = cv::Point(static_cast<int>(isection.x), static_cast<int>(isection.y));
    cv::Point movement = curLoc - svoCurPosition;

    float distance = static_cast<float>(std::sqrt(std::pow(movement.x, 2.f) + std::pow(movement.y, 2.f)));
    movement = cv::Point2f(
            static_cast<float>(std::sin(direction) * distance),
            static_cast<float>(-std::cos(direction) * distance)
    );

    svoCurPosition = curLoc;
    return movement;
}

#include "WorkspaceRuntime.hpp"

#include <iostream>

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
    currentScale = scaleModel.updateScale(
            1.0f,
            static_cast<float>(metadata.altitude),
            templ.cols,
            [this](float minScale, float maxScale) {
                core.setScale(minScale, maxScale);
            }
    );
    startLocation = core.getFilter()->getPredictedLocation();
    std::cout << " done!" << std::endl;
}

void WorkspaceRuntime::update(const MetadataEntry &metadata) {
    cv::Point movement = motionModel.getMovementFromSvo(metadata, svoCoordinates, direction, svoCurPosition);
    currentScale = scaleModel.updateScale(
            1.0f,
            static_cast<float>(metadata.altitude),
            640,
            [this](float minScale, float maxScale) {
                core.setScale(minScale, maxScale);
            }
    );
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

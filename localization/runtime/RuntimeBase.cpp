#include "RuntimeBase.hpp"

#include <iostream>

void RuntimeBase::initialize(const MetadataEntry &metadata, const ParticleFilterConfig &config) {
    config.validate();
    std::cout << "Initializing...";
    std::cout.flush();
    direction_ = metadata.imuOrientation.toRPY().getZ();
    svoCurPosition_ = metadata.mapLocation;
    svoCoordinates_ = std::make_shared<GeographicLib::LocalCartesian>(
            metadata.latitude,
            metadata.longitude,
            metadata.altitude
    );
    core_->initialize(metadata, config);
    core_->setDirection(direction_);
    cv::Mat templ = metadata.getImageColored();
    currentScale_ = scaleModel_.updateScale(
            1.0f,
            static_cast<float>(metadata.altitude),
            templ.cols,
            [this](float minScale, float maxScale) {
                core_->setScale(minScale, maxScale);
            }
    );
    startLocation_ = core_->getFilter()->getPredictedLocation();
    std::cout << " done!" << std::endl;
}

void RuntimeBase::update(const MetadataEntry &metadata) {
    auto svoResult = motionModel_.getMovementFromSvo(metadata, svoCoordinates_, direction_, svoCurPosition_);
    svoCurPosition_ = svoResult.updatedPosition;
    cv::Point movement = svoResult.movement;
    cv::Mat templ = metadata.getImageColored();
    currentScale_ = scaleModel_.updateScale(
            1.0f,
            static_cast<float>(metadata.altitude),
            templ.cols,
            [this](float minScale, float maxScale) {
                core_->setScale(minScale, maxScale);
            }
    );
    direction_ = metadata.imuOrientation.toRPY().getZ();
    core_->setDirection(direction_);
    core_->setTemplate(templ);
    if(!affineMatching_) {
        corners_ = core_->filterParticles(movement, bestTransform_);
        bestView_ = core_->getBestParticleView(metadata.map);
    } else {
#ifdef USE_CV_GPU
        corners_ = core_->filterParticlesAffine(movement, bestTransform_);
#else
        throw std::runtime_error("Affine particle matching is available with GPU support only");
#endif
    }
}

bool RuntimeBase::isAffineMatching() const {
    return affineMatching_;
}

void RuntimeBase::setAffineMatching(bool affineMatching) {
    affineMatching_ = affineMatching;
}

void RuntimeBase::setCorrelationLowBound(float bound) {
    core_->setLowBound(bound);
}

void RuntimeBase::setConversionMethod(ParticleFastMatch::ConversionMode method) {
    core_->setConversionMethod(method);
}

void RuntimeBase::describe() const {
    core_->describe();
}

const Particles &RuntimeBase::getParticles() const {
    return core_->getParticles();
}

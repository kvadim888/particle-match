#pragma once

#include <memory>
#include <sstream>
#include <stdexcept>

#include <GeographicLib/LocalCartesian.hpp>
#include <fastmatch-dataset/MetadataEntry.hpp>

#include "IRuntime.hpp"
#include "core/ParticleFilterCore.hpp"
#include "models/MotionModelSvo.hpp"
#include "models/ScaleModel.hpp"

class RuntimeBase : public IRuntime {
public:
    RuntimeBase() : core_(std::make_unique<ParticleFilterCore>()) {}
    explicit RuntimeBase(std::unique_ptr<ParticleFilterCore> core) : core_(std::move(core)) {}

    void initialize(const MetadataEntry &metadata, const ParticleFilterConfig &config) override;

    void update(const MetadataEntry &metadata) override;

    bool isAffineMatching() const override;

    void setAffineMatching(bool affineMatching) override;

    void setCorrelationLowBound(float bound) override;

    void setConversionMethod(ParticleFastMatch::ConversionMode method) override;

    void describe() const override;

    const Particles &getParticles() const override;

protected:
    bool affineMatching_ = false;
    std::unique_ptr<ParticleFilterCore> core_;
    cv::Point svoCurPosition_;
    double direction_ = 0.0;
    cv::Point startLocation_;
    cv::Mat bestTransform_;
    float currentScale_ = 0.0f;
    std::shared_ptr<GeographicLib::LocalCartesian> svoCoordinates_;
    MotionModelSvo motionModel_;
    ScaleModel scaleModel_;

    std::vector<cv::Point> corners_;
    cv::Mat bestView_;
};

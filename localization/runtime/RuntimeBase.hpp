#pragma once

#include <memory>
#include <sstream>
#include <stdexcept>

#include <GeographicLib/LocalCartesian.hpp>
#include <fastmatch-dataset/MetadataEntry.hpp>

#include "core/ParticleFilterConfig.hpp"
#include "core/ParticleFilterCore.hpp"
#include "models/MotionModelSvo.hpp"
#include "models/ScaleModel.hpp"

class RuntimeBase {
public:
    virtual ~RuntimeBase() = default;

    void initialize(const MetadataEntry &metadata, const ParticleFilterConfig &config);

    void update(const MetadataEntry &metadata);

    virtual bool preview(const MetadataEntry &metadata, const cv::Mat &image, std::stringstream &stringOutput) = 0;

    bool isAffineMatching() const;

    void setAffineMatching(bool affineMatching);

    virtual bool isDisplayImage() const = 0;

    virtual void setDisplayImage(bool displayImage) = 0;

    virtual void setWriteImageToDisk(bool writeImageToDisk) = 0;

    virtual void setOutputDirectory(const std::string &outputDirectory) = 0;

    void setCorrelationLowBound(float bound);

    void setConversionMethod(ParticleFastMatch::ConversionMode method);

    void describe() const;

    const Particles &getParticles() const;

protected:
    bool affineMatching_ = false;
    ParticleFilterCore core_;
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

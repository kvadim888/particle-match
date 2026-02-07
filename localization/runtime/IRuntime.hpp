#pragma once

#include <sstream>
#include <string>

#include <fastmatch-dataset/MetadataEntry.hpp>
#include <src/ParticleFastMatch.hpp>

#include "core/ParticleFilterConfig.hpp"

class IRuntime {
public:
    virtual ~IRuntime() = default;

    virtual void initialize(const MetadataEntry &metadata, const ParticleFilterConfig &config) = 0;
    virtual void update(const MetadataEntry &metadata) = 0;
    virtual bool preview(const MetadataEntry &metadata, const cv::Mat &image, std::stringstream &stringOutput) = 0;

    virtual bool isAffineMatching() const = 0;
    virtual void setAffineMatching(bool affineMatching) = 0;

    virtual bool isDisplayImage() const = 0;
    virtual void setDisplayImage(bool displayImage) = 0;

    virtual void setWriteImageToDisk(bool writeImageToDisk) = 0;
    virtual void setOutputDirectory(const std::string &outputDirectory) = 0;

    virtual void setCorrelationLowBound(float bound) = 0;
    virtual void setConversionMethod(ParticleFastMatch::ConversionMode method) = 0;

    virtual void describe() const = 0;
    virtual const Particles &getParticles() const = 0;
};

#pragma once

#include <memory>
#include <vector>

#include <GeographicLib/LocalCartesian.hpp>
#include <fastmatch-dataset/MetadataEntry.hpp>

#include "core/ParticleFilterConfig.hpp"
#include "core/ParticleFilterCore.hpp"
#include "io/PreviewRenderer.hpp"

class WorkspaceRuntime {
public:
    void initialize(const MetadataEntry &metadata, const ParticleFilterConfig &config);

    void update(const MetadataEntry &metadata);

    bool preview(const MetadataEntry &metadata, const cv::Mat &image, std::stringstream &stringOutput);

    bool isAffineMatching() const;

    void setAffineMatching(bool affineMatching);

    bool isDisplayImage() const;

    void setDisplayImage(bool displayImage);

    void setWriteImageToDisk(bool writeImageToDisk);

    void setOutputDirectory(const std::string &outputDirectory);

    void setCorrelationLowBound(float bound);

    void setConversionMethod(ParticleFastMatch::ConversionMode method);

    void describe() const;

    const Particles &getParticles() const;

    cv::Point getMovementFromSvo2(const MetadataEntry &metadata);

private:
    void updateScale(float hfov, float altitude, uint32_t imageWidth);

    cv::Point getMovementFromSvo(const MetadataEntry &metadata);

    bool affineMatching = false;
    ParticleFilterCore core;
    PreviewRenderer renderer;
    cv::Point svoCurPosition;
    double direction = 0.0;
    cv::Point startLocation;
    cv::Mat map;
    std::vector<cv::Point> corners;
    cv::Mat bestTransform;
    cv::Mat bestView;
    float currentScale = 0.0f;
    std::shared_ptr<GeographicLib::LocalCartesian> svoCoordinates;
};

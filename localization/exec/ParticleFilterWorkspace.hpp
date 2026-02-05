//
// Created by rokas on 17.12.4.
//

#pragma once

#include <vector>
#include <src/ParticleFastMatch.hpp>
#include <fastmatch-dataset/MetadataEntry.hpp>
#include <GeographicLib/LocalCartesian.hpp>

#include "core/ParticleFilterConfig.hpp"
#include "io/PreviewRenderer.hpp"
#include "models/MotionModelSvo.hpp"
#include "models/ScaleModel.hpp"

class ParticleFilterWorkspace {
protected:
    bool affineMatching = false;
    std::shared_ptr<ParticleFastMatch> pfm;
    cv::Point svoCurPosition;
    double direction;
    cv::Point startLocation;
    cv::Mat map;
    std::vector<cv::Point> corners;
    cv::Mat bestTransform;
    float currentScale = 0.0;
    std::shared_ptr<GeographicLib::LocalCartesian> svoCoordinates;
    MotionModelSvo motionModel;
    ScaleModel scaleModel;
    PreviewRenderer renderer;

public:
    bool isDisplayImage() const;

    void setDisplayImage(bool displayImage);

    void setWriteImageToDisk(bool writeImageToDisk);

    void setOutputDirectory(const string &outputDirectory);

    void initialize(const MetadataEntry &metadata, const ParticleFilterConfig &config);

    void update(const MetadataEntry &metadata);

    bool preview(const MetadataEntry &metadata, cv::Mat image, std::stringstream &stringOutput) const;

    Mat bestView;

    bool isAffineMatching() const;

    void setAffineMatching(bool affineMatching);

    void setCorrelationLowBound(float bound);

    void setConversionMethod(ParticleFastMatch::ConversionMode method);

    void describe() const;

    const Particles &getParticles() const;

};

//
// Created by rokas on 17.12.4.
//

#pragma once

#include <vector>
#include <src/ParticleFastMatch.hpp>
#include <fastmatch-dataset/MetadataEntry.hpp>
#include <GeographicLib/LocalCartesian.hpp>

#include "core/ParticleFilterConfig.hpp"
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
    std::string outputDirectory;
    cv::Mat bestTransform;
    float currentScale = 0.0;
    bool writeImageToDisk = false;
    bool displayImage = true;
    std::shared_ptr<GeographicLib::LocalCartesian> svoCoordinates;
    MotionModelSvo motionModel;
    ScaleModel scaleModel;

    static void visualizeGT(const cv::Point &loc, double yaw, cv::Mat &image, int radius, int thickness,
                            const cv::Scalar &color = CV_RGB(255, 255, 0));

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

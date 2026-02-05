#pragma once

#include <memory>
#include <vector>

#include <fastmatch-dataset/MetadataEntry.hpp>
#include <src/ParticleFastMatch.hpp>

#include "ParticleFilterConfig.hpp"

class ParticleFilterCore {
public:
    void initialize(const MetadataEntry &metadata, const ParticleFilterConfig &config);

    void setDirection(double direction);

    void setTemplate(const cv::Mat &templ);

    void setImage(const cv::Mat &image);

    void setScale(float minScale, float maxScale, uint32_t searchSteps = 5);

    std::vector<cv::Point> filterParticles(const cv::Point2f &movement, cv::Mat &bestTransform);

    std::vector<cv::Point> filterParticlesAffine(const cv::Point2f &movement, cv::Mat &bestTransform);

    cv::Mat getBestParticleView(const cv::Mat &map) const;

    void setLowBound(float bound);

    void setConversionMethod(ParticleFastMatch::ConversionMode method);

    void describe() const;

    const Particles &getParticles() const;

    std::shared_ptr<ParticleFastMatch> getFilter() const;

private:
    std::shared_ptr<ParticleFastMatch> pfm;
};

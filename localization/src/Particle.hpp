//
// Created by rokas on 17.6.20.
//

#pragma once

#include <FAsT-Match/MatchConfig.h>
#include "ParticleConfig.hpp"

class Particle {
private:
    std::shared_ptr<std::vector<float>> s_initial = std::make_shared<std::vector<float>>();
    std::shared_ptr<ParticleConfig> config;

public:
    double getDirection() const;
    std::vector<bool> insiders;

protected:
    std::vector<fast_match::MatchConfig> configs;
    float probability;
    float samplingFactor;
    float accumulatedProbability = 0.f;
    std::vector<float> oldProbabilities = {};
    uint32_t iteration = 0;
    float weight;
    cv::Mat bestTransform;
    float correlation = -1.0f;
public:
    float getCorrelation() const;

    void setCorrelation(float correlation);

protected:
    std::vector<cv::Mat> getAffines(const cv::Size& imageSize, const cv::Size& templSize);

public:
    int x, y;

    const cv::Mat &getBestTransform() const;
    void setS_initial(const std::shared_ptr<std::vector<float>> &s_initial);
    void setConfig(const std::shared_ptr<ParticleConfig>& config);
    double evaluate(cv::Mat& image, cv::Mat& templ, cv::Mat& xs, cv::Mat& ys);
    bool operator<(const Particle& str) const;
    bool operator>(const Particle& str) const;
    std::string serialize(int binSize);
    float getWeight() const;
    void setWeight(float weight);
    float getSamplingFactor() const;
    void setSamplingFactor(float samplingFactor);
    float getProbability() const;
    void setProbability(float probability);
    void setMinimalProbability(float probability);
    void setMaximalProbability(float probability);
    Particle(int x, int y, const std::shared_ptr<ParticleConfig>& config);
    Particle(const Particle& a);
    Particle(Particle&& a) noexcept = default;
    Particle& operator=(const Particle& a) = default;
    Particle& operator=(Particle&& a) noexcept = default;
    const std::vector<fast_match::MatchConfig> & getConfigs(int id);
    void propagate(const cv::Point2f& movement);
    void updateConfigs();
    cv::Point2i getLocationInMapCoords() const;
    cv::Point2i toPoint() const;
    cv::Mat staticTransformation() const;

    double getDirectionDegrees() const;

    float getScale() const;

    cv::Mat mapTransformation() const;

    cv::Mat getMapImage(const cv::Mat& map, const cv::Size& imsize) const;

    std::vector<cv::Point> getCorners() const;
};

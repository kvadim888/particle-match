#include "ParticleFilterCore.hpp"

#include <iostream>

void ParticleFilterCore::initialize(const MetadataEntry &metadata) {
    pfm = std::make_shared<ParticleFastMatch>(
            metadata.mapLocation, // startLocation
            metadata.map.size(), // mapSize
            500, // radius
            .1f, // epsilon
            200, // particleCount
            .99, // quantile_
            .5, // kld_error_
            5, // bin_size_
            true // use_gaussian
    );
    cv::Mat templ = metadata.getImageColored();
    pfm->setTemplate(templ);
    pfm->setImage(metadata.map);
}

void ParticleFilterCore::setDirection(double direction) {
    pfm->setDirection(direction);
}

void ParticleFilterCore::setTemplate(const cv::Mat &templ) {
    pfm->setTemplate(templ);
}

void ParticleFilterCore::setImage(const cv::Mat &image) {
    pfm->setImage(image);
}

void ParticleFilterCore::setScale(float minScale, float maxScale, uint32_t searchSteps) {
    pfm->setScale(minScale, maxScale, searchSteps);
}

std::vector<cv::Point> ParticleFilterCore::filterParticles(const cv::Point2f &movement, cv::Mat &bestTransform) {
    return pfm->filterParticles(movement, bestTransform);
}

std::vector<cv::Point> ParticleFilterCore::filterParticlesAffine(const cv::Point2f &movement, cv::Mat &bestTransform) {
    return pfm->filterParticlesAffine(movement, bestTransform);
}

cv::Mat ParticleFilterCore::getBestParticleView(const cv::Mat &map) const {
    return pfm->getBestParticleView(map);
}

void ParticleFilterCore::setLowBound(float bound) {
    pfm->setLowBound(bound);
}

void ParticleFilterCore::setConversionMethod(ParticleFastMatch::ConversionMode method) {
    pfm->conversionMode = method;
}

void ParticleFilterCore::describe() const {
    std::cout << "Using conversion mode: " << pfm->conversionModeString() << "\n";
    std::cout << "Conversion bound: " << pfm->getLowBound() << "\n";
}

const Particles &ParticleFilterCore::getParticles() const {
    return pfm->getParticles();
}

std::shared_ptr<ParticleFastMatch> ParticleFilterCore::getFilter() const {
    return pfm;
}

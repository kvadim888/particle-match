//
// Created by rokas on 17.12.4.
//

#include <boost/filesystem/path.hpp>
#include "ParticleFilterWorkspace.hpp"

namespace fs = boost::filesystem;

void ParticleFilterWorkspace::initialize(const MetadataEntry &metadata, const ParticleFilterConfig &config) {
    std::cout << "Initializing...";
    std::cout.flush();
    direction = metadata.imuOrientation.toRPY().getZ();
    svoCurPosition = metadata.mapLocation;
    svoCoordinates = std::make_shared<GeographicLib::LocalCartesian>(
            metadata.latitude,
            metadata.longitude,
            metadata.altitude
    );
    pfm = std::make_shared<ParticleFastMatch>(
            svoCurPosition, // startLocation
            metadata.map.size(), // mapSize
            config.radius, // radius
            config.epsilon, // epsilon
            config.particleCount, // particleCount
            config.quantile, // quantile_
            config.kld_error, // kld_error_
            config.binSize, // bin_size_
            config.use_gaussian // use_gaussian
    );
    pfm->setDirection(direction);
    cv::Mat templ = metadata.getImageColored();
    pfm->setTemplate(templ);
    pfm->setImage(metadata.map);
    map = metadata.map;
    currentScale = scaleModel.updateScale(
            1.0f,
            static_cast<float>(metadata.altitude),
            templ.cols,
            [this](float minScale, float maxScale) {
                pfm->setScale(minScale, maxScale);
            }
    );
    startLocation = pfm->getPredictedLocation();
    std::cout << " done!" << std::endl;
}

void ParticleFilterWorkspace::update(const MetadataEntry &metadata) {
    cv::Point movement = motionModel.getMovementFromSvo(metadata, svoCoordinates, direction, svoCurPosition);
    currentScale = scaleModel.updateScale(
            1.0f,
            static_cast<float>(metadata.altitude),
            640,
            [this](float minScale, float maxScale) {
                pfm->setScale(minScale, maxScale);
            }
    );
    direction = metadata.imuOrientation.toRPY().getZ();
    pfm->setDirection(direction);
    cv::Mat templ = metadata.getImageColored();
    pfm->setTemplate(templ);
    if(!affineMatching) {
        corners = pfm->filterParticles(movement, bestTransform);
        bestView = pfm->getBestParticleView(metadata.map);
    } else {
#ifdef USE_CV_GPU
        corners = pfm->filterParticlesAffine(movement, bestTransform);
#else
        throw std::runtime_error("Affine particle matching is available with GPU support only");
#endif
    }
}

bool ParticleFilterWorkspace::preview(const MetadataEntry &metadata, cv::Mat planeView, std::stringstream& stringOutput)
const {
    return renderer.render(
            metadata,
            planeView,
            stringOutput,
            map,
            startLocation,
            svoCurPosition,
            direction,
            corners,
            bestTransform,
            bestView,
            pfm
    );
}


void ParticleFilterWorkspace::setWriteImageToDisk(bool writeImageToDisk) {
    renderer.setWriteImageToDisk(writeImageToDisk);
}

void ParticleFilterWorkspace::setOutputDirectory(const string &outputDirectory) {
    renderer.setOutputDirectory(outputDirectory);
}

bool ParticleFilterWorkspace::isAffineMatching() const {
    return affineMatching;
}

void ParticleFilterWorkspace::setAffineMatching(bool affineMatching) {
    ParticleFilterWorkspace::affineMatching = affineMatching;
}

bool ParticleFilterWorkspace::isDisplayImage() const {
    return renderer.isDisplayImage();
}

void ParticleFilterWorkspace::setDisplayImage(bool displayImage) {
    renderer.setDisplayImage(displayImage);
}

void ParticleFilterWorkspace::setCorrelationLowBound(float bound) {
    pfm->setLowBound(bound);
}

void ParticleFilterWorkspace::setConversionMethod(ParticleFastMatch::ConversionMode method) {
    pfm->conversionMode = method;
}

void ParticleFilterWorkspace::describe() const {
    std::cout << "Using conversion mode: " << pfm->conversionModeString() << "\n";
    std::cout << "Conversion bound: " << pfm->getLowBound() << "\n";
}

const Particles &ParticleFilterWorkspace::getParticles() const {
    return pfm->getParticles();
}

//
// Created by rokas on 17.6.20.
//

#include <src/Utilities.hpp>
#include <src/FastMatch.hpp>
#include "Particle.hpp"
#include "GeometryUtils.hpp"

// Default image center used for coordinate transformations
static constexpr int kDefaultHalfWidth = 320;
static constexpr int kDefaultHalfHeight = 240;

// Direction calibration offset in degrees
static constexpr float kDirectionOffsetDeg = 75.0f;

float Particle::getProbability() const {
    return probability;
}

void Particle::setProbability(float probability) {
    accumulatedProbability += probability;
    oldProbabilities.push_back(probability);
    if (oldProbabilities.size() >= 5) {
        accumulatedProbability -= oldProbabilities[0];
        oldProbabilities.erase(oldProbabilities.begin());
    } else {
        iteration++;
    }
    Particle::probability = accumulatedProbability / iteration;
}

Particle::Particle(int x, int y, const std::shared_ptr<ParticleConfig>& config)
    : config(config), x(x), y(y), probability(1.0) {
    updateConfigs();
}

const std::vector<fast_match::MatchConfig> &Particle::getConfigs(int id) {
    for (auto &cfg : configs) {
        cfg.setId(id);
    }
    return configs;
}

void Particle::propagate(const cv::Point2f &movement) {

    cv::Point2f m = movement;
    float alpha = 4.0f;
    float min_movement = 10.0f;
    float min_noise_level = 5.0f;
    // This is the case when the odometry is lost
    if (movement.x == 0.f && movement.y == 0.f) {
        m.x = static_cast<float>(Utilities::gausian_noise(1) * min_movement * alpha);
        m.y = static_cast<float>(Utilities::gausian_noise(1) * min_movement * alpha);
    } else {
        // Do not trust in noise level measurements
        if (movement.x < min_noise_level) {
            m.x += Utilities::gausian_noise(alpha) * min_noise_level;
        } else {
            m.x += Utilities::gausian_noise(alpha) * m.x;
        }
        if (movement.y < min_noise_level) {
            m.y += Utilities::gausian_noise(alpha) * min_noise_level;
        } else {
            m.y += Utilities::gausian_noise(alpha) * m.y;
        }
    }
    x += m.x;
    y += m.y;
    updateConfigs();
}

void Particle::updateConfigs() {
    configs.clear();
    auto scale_steps = static_cast<int>(s_initial->size());
    auto rotation_steps = static_cast<int>(config->r_initial.size());

    std::vector<float> r2_rotations = {
            -(3 * config->r_step),
            0,
            3 * config->r_step,
    };
    auto nr2_steps = r2_rotations.size();

    auto rotations = config->r_initial;
    for (float &rotation : rotations) {
        rotation += config->direction;
    }

    for (size_t sx = 0; sx < static_cast<size_t>(scale_steps); sx++) {
        for (size_t sy = 0; sy < static_cast<size_t>(scale_steps); sy++) {
            for (int r1 = 0; r1 < rotation_steps; r1++) {
                for (size_t r2 = 0; r2 < nr2_steps; r2++) {
                    configs.emplace_back(
                            x - config->mapCenter.x,
                            y - config->mapCenter.y,
                            r2_rotations[r2],
                            s_initial->at(sx),
                            s_initial->at(sy),
                            rotations[r1]
                    );
                }
            }
        }
    }
}

double Particle::evaluate(cv::Mat &image, cv::Mat &templ, cv::Mat &xs, cv::Mat &ys) {
    std::vector<cv::Mat> affines = getAffines(image.size(), templ.size());
    /* For the configs, calculate the scores / distances */
    std::vector<double> distances = fast_match::FAsTMatch::evaluateConfigs(image, templ, affines, xs, ys, true);
    /* Find the minimum distance */
    auto min_itr = min_element(distances.begin(), distances.end());
    int min_index = static_cast<int>(min_itr - distances.begin());
    double best_distance = distances[min_index];
    bestTransform = configs[min_index].getAffineMatrix();
    setProbability(static_cast<float>(best_distance));
    return best_distance;
}

std::vector<cv::Mat> Particle::getAffines(const cv::Size &imageSize, const cv::Size &templSize) {
    std::vector<bool> insiders;
    std::vector<cv::Mat> affines = Utilities::configsToAffine(configs, insiders, imageSize, templSize);

    /* Filter out configurations that fall outside of the boundaries */
    /* the internal logic of configsToAffine has more information */
    std::vector<fast_match::MatchConfig> temp_configs;
    for (size_t i = 0; i < insiders.size(); i++)
        if (insiders[i])
            temp_configs.push_back(configs[i]);
    configs = temp_configs;
    return affines;
}

const cv::Mat &Particle::getBestTransform() const {
    return bestTransform;
}

void Particle::setMinimalProbability(float probability) {
    if (Particle::probability > probability) {
        Particle::probability = probability;
    }
}

void Particle::setMaximalProbability(float probability) {
    if (Particle::probability < probability) {
        Particle::probability = probability;
    }
}

float Particle::getSamplingFactor() const {
    return samplingFactor;
}

void Particle::setSamplingFactor(float samplingFactor) {
    Particle::samplingFactor = samplingFactor;
}

bool Particle::operator<(const Particle &str) const {
    return samplingFactor < str.samplingFactor;
}

bool Particle::operator>(const Particle &str) const {
    return samplingFactor > str.samplingFactor;
}

float Particle::getWeight() const {
    return weight;
}

void Particle::setWeight(float weight) {
    Particle::weight = weight;
}

std::string Particle::serialize(int binSize) {
    return std::to_string(x - (x % binSize)) + "x" + std::to_string(y - (y % binSize));
}

void Particle::setS_initial(const std::shared_ptr<std::vector<float>> &s_initial) {
    Particle::s_initial = s_initial;
}

cv::Point2i Particle::getLocationInMapCoords() const {
    return {x - config->mapCenter.x, y - config->mapCenter.y};
}

double Particle::getDirection() const {
    return config->direction;
}

double Particle::getDirectionDegrees() const {
    return config->direction * geometry::kRadToDeg;
}

cv::Point2i Particle::toPoint() const {
    return cv::Point2i(x, y);
}

cv::Mat Particle::staticTransformation() const {
    cv::Mat T = cv::getRotationMatrix2D(cv::Point(kDefaultHalfWidth, kDefaultHalfHeight), getDirectionDegrees(), getScale());
    return T;
}

cv::Mat Particle::mapTransformation() const {
    return cv::getRotationMatrix2D(toPoint(), getDirectionDegrees() - kDirectionOffsetDeg, getScale());
}

float Particle::getScale() const {
    return (*s_initial)[2];
}

float Particle::getCorrelation() const {
    return correlation;
}

void Particle::setCorrelation(float correlation) {
    Particle::correlation = correlation;
}

cv::Mat Particle::getMapImage(const cv::Mat &map, const cv::Size &imsize) const {

    assert(map.type() == CV_8UC1);

    cv::Mat T = mapTransformation();
    double m11 = T.at<double>(0, 0),
            m12 = T.at<double>(0, 1),
            m13 = T.at<double>(0, 2),
            m21 = T.at<double>(1, 0),
            m22 = T.at<double>(1, 1),
            m23 = T.at<double>(1, 2);
    cv::Mat preview(imsize, CV_8UC1, cv::Scalar(255));
    int halfwidth = imsize.width / 2;
    int halfheight = imsize.height / 2;
    for (int y_ = -halfheight, cy = 0; y_ < halfheight; y_++, cy++) {
        for (int x_ = -halfwidth, cx = 0; x_ < halfwidth; x_++, cx++) {
            cv::Point2i p(x_ + x, y_ + y);
            // Transform points
            cv::Point2i pTran(
                    m11 * p.x + m12 * p.y + m13,
                    m21 * p.x + m22 * p.y + m23
            );
            if (pTran.x >= 0 && pTran.x < map.cols && pTran.y >= 0 && pTran.y < map.rows) {
                preview.at<uint8_t>(cv::Point2i(cx, cy)) = map.at<uint8_t>(pTran);
            }
        }
    }
    return preview;
}

std::vector<cv::Point> Particle::getCorners() const {
    cv::Mat T = mapTransformation();
    double m11 = T.at<double>(0, 0),
            m12 = T.at<double>(0, 1),
            m13 = T.at<double>(0, 2),
            m21 = T.at<double>(1, 0),
            m22 = T.at<double>(1, 1),
            m23 = T.at<double>(1, 2);
    return {
            cv::Point2i(
                    m11 * (x - kDefaultHalfWidth) + m12 * (y - kDefaultHalfHeight) + m13,
                    m21 * (x - kDefaultHalfWidth) + m22 * (y - kDefaultHalfHeight) + m23
            ),
            cv::Point2i(
                    m11 * (x + kDefaultHalfWidth) + m12 * (y - kDefaultHalfHeight) + m13,
                    m21 * (x + kDefaultHalfWidth) + m22 * (y - kDefaultHalfHeight) + m23
            ),
            cv::Point2i(
                    m11 * (x + kDefaultHalfWidth) + m12 * (y + kDefaultHalfHeight) + m13,
                    m21 * (x + kDefaultHalfWidth) + m22 * (y + kDefaultHalfHeight) + m23
            ),
            cv::Point2i(
                    m11 * (x - kDefaultHalfWidth) + m12 * (y + kDefaultHalfHeight) + m13,
                    m21 * (x - kDefaultHalfWidth) + m22 * (y + kDefaultHalfHeight) + m23
            )
    };
}

Particle::Particle(const Particle &a) = default;

void Particle::setConfig(const std::shared_ptr<ParticleConfig>& cfg) {
    config = cfg;
}

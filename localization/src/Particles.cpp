//
// Created by rokas on 17.6.20.
//

#include <src/Utilities.hpp>
#include "Particles.hpp"

void Particles::init(cv::Point2i startLocation, const cv::Size mapSize,  double radius, int particleCount, bool use_gaussian) {
    double r, a;
    int size = 0;
    particleConfig->setMapDimensions(mapSize);
    while (size < particleCount) {
        if(use_gaussian) {
            a = ((Utilities::gausian_noise(1.0f) - 0.5) * 2) * 2 * M_PI;
            double u = Utilities::gausian_noise(radius) + Utilities::gausian_noise(radius);
            r = u > radius ? (2 * radius) - u : u;
        } else {
            a = ((Utilities::uniform_dist() - 0.5) * 2) * 2 * M_PI;
            double r1 = Utilities::uniform_dist() * radius,
                    r2 = Utilities::uniform_dist() * radius;
            double u = r1 + r2;
            r = u > radius ? (2 * radius) - u : u;
        }
        auto x = static_cast<int>(startLocation.x + (r * cos(a)));
        auto y = static_cast<int>(startLocation.y + (r * sin(a)));
        // Skip duplicate particles
        if(!isLocationOccupied(x, y)) {
            addParticle(x, y);
            data_.back().setProbability(.5f);
            size++;
        }
    }
    normalize();
}

void Particles::addParticle(int x, int y) {
    data_.emplace_back(x, y, particleConfig);
}

void Particles::addParticle(Particle p) {
    data_.emplace_back(std::move(p));
}

std::vector<fast_match::MatchConfig> Particles::getConfigs() {
    std::vector<fast_match::MatchConfig> configs;
    int i = 0;
    for (auto &it : data_) {
        auto& curConfigs = it.getConfigs(i++);
        configs.insert(configs.end(), curConfigs.begin(), curConfigs.end());
    }
    return configs;
}

bool Particles::isLocationOccupied(int x, int y) {
    for (const auto& it : data_) {
        if(it.x == x && it.y == y) {
            return true;
        }
    }
    return false;
}

void Particles::propagate(const cv::Point2f &movement, float alpha) {
    std::uniform_real_distribution<float> dist(-1.f, 1.f);
    for (auto &it : data_) {
        it.propagate(cv::Point2f(
                movement.x * alpha * dist(rng_),
                movement.y * alpha * dist(rng_)
        ));
    }
}

void Particles::printProbabilities() {
    int i = 1;
    for (const auto& it : data_) {
        std::cout << "Particle no " << i++ << " probability is: " << it.getProbability() << "\n";
    }
}

std::vector<cv::Point> Particles::evaluate(cv::Mat image, cv::Mat templ, int no_of_points) {
    /* Randomly sample points */
    cv::Mat xs(1, no_of_points, CV_32SC1),
            ys(1, no_of_points, CV_32SC1);
    std::uniform_int_distribution<int> xDist(1, templ.cols - 1);
    std::uniform_int_distribution<int> yDist(1, templ.rows - 1);
    for (int i = 0; i < no_of_points; i++) {
        xs.at<int>(0, i) = xDist(rng_);
        ys.at<int>(0, i) = yDist(rng_);
    }

    double lowestDistance = +INFINITY;
    cv::Mat bestTrasform;
    for (auto &it : data_) {
        double distance = it.evaluate(image, templ, xs, ys);
        if(distance < lowestDistance) {
            lowestDistance = distance;
            bestTrasform = it.getBestTransform();
        }
    }
    return Utilities::calcCorners(image.size(), templ.size(), bestTrasform);
}

Particle Particles::sample() {
    double sampleThreshold = Utilities::uniform_dist();
    if(sampleThreshold < .5f) {
        for (const auto &it : data_) {
            if (it.getSamplingFactor() > sampleThreshold) {
                return Particle(it);
            }
        }
    } else {
        for (auto i = data_.rbegin(); i != data_.rend(); ++i ) {
            if (i->getSamplingFactor() < sampleThreshold) {
                return Particle(*i);
            }
        }
    }
    // Fallback: return last particle if no threshold matched
    return Particle(data_.back());
}

void Particles::sortAscending() {
    std::sort(data_.begin(), data_.end(), std::greater<>());
}


void Particles::normalize() {
    float normalizationFactor = 0.f,
            total = 0.f;
    for (const auto &it : data_) {
        normalizationFactor += it.getProbability();
    }
    for (auto &it : data_) {
        float weight = it.getProbability() / normalizationFactor;
        it.setWeight(weight);
        total += weight;
        it.setSamplingFactor(1 - total);
    }
}

cv::Point2i Particles::getWeightedSum() const {
    double s_x = .0, s_y = .0;
    for(const auto& it : data_) {
        s_x += it.x * it.getWeight();
        s_y += it.y * it.getWeight();
    }
    return cv::Point2i(static_cast<int>(s_x), static_cast<int>(s_y));
}

void Particles::setScale(float min, float max, uint32_t steps) {
    float delta = (std::abs(min - max)) / static_cast<float>(steps - 1);
    s_initial = std::make_shared<std::vector<float>>();
    for(uint32_t i = 0; i < steps; i++) {
        s_initial->push_back(min + (i * delta));
    }
    for(auto& p : data_) {
        p.setS_initial(s_initial);
    }
}

//
// Created by rokas on 17.6.20.
//

#pragma once

#include <vector>
#include <memory>
#include <algorithm>
#include <random>
#include <opencv2/core/types.hpp>
#include <opencv2/core/mat.hpp>
#include <FAsT-Match/MatchConfig.h>
#include "Particle.hpp"
#include "ParticleConfig.hpp"

class Particles {
public:
    // Vector interface forwarding
    using iterator = std::vector<Particle>::iterator;
    using const_iterator = std::vector<Particle>::const_iterator;
    using reverse_iterator = std::vector<Particle>::reverse_iterator;
    using const_reverse_iterator = std::vector<Particle>::const_reverse_iterator;

    iterator begin() { return data_.begin(); }
    iterator end() { return data_.end(); }
    const_iterator begin() const { return data_.begin(); }
    const_iterator end() const { return data_.end(); }
    reverse_iterator rbegin() { return data_.rbegin(); }
    reverse_iterator rend() { return data_.rend(); }
    const_reverse_iterator rbegin() const { return data_.rbegin(); }
    const_reverse_iterator rend() const { return data_.rend(); }

    Particle& operator[](size_t i) { return data_[i]; }
    const Particle& operator[](size_t i) const { return data_[i]; }
    Particle& front() { return data_.front(); }
    const Particle& front() const { return data_.front(); }
    Particle& back() { return data_.back(); }
    const Particle& back() const { return data_.back(); }

    size_t size() const { return data_.size(); }
    bool empty() const { return data_.empty(); }

    template<typename InputIt>
    void assign(InputIt first, InputIt last) { data_.assign(first, last); }

    // Domain-specific methods
    void init(cv::Point2i startLocation, const cv::Size mapSize, double radius, int particleCount, bool use_gaussian);
    std::vector<fast_match::MatchConfig> getConfigs();
    void propagate(const cv::Point2f& movement, float alpha = 2.f);

    void addParticle(Particle p);

    const std::shared_ptr<ParticleConfig>& getConfig() const { return particleConfig; }

    std::vector<cv::Point> evaluate(cv::Mat image, cv::Mat templ, int no_of_points);

    void printProbabilities();

    Particle sample();

    void normalize();

    void sortAscending();

    cv::Point2i getWeightedSum() const;

    void setScale(float min, float max, uint32_t steps = 5);

protected:
    std::vector<Particle> data_;
    std::shared_ptr<ParticleConfig> particleConfig = std::make_shared<ParticleConfig>();
    std::shared_ptr<std::vector<float>> s_initial = std::make_shared<std::vector<float>>();

    std::mt19937 rng_{std::random_device{}()};

    void addParticle(int x, int y);

    bool isLocationOccupied(int x, int y);
};

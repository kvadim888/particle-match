//
//  FAsTMatch.h
//  FAsT-Match
//
//  Created by Saburo Okita on 23/05/14.
//  Copyright (c) 2014 Saburo Okita. All rights reserved.
//

#pragma once

#include <iostream>
#include <opencv2/opencv.hpp>
#include <iterator>
#include <boost/shared_ptr.hpp>

#include "../FAsT-Match/MatchNet.h"
#include "../FAsT-Match/MatchConfig.h"
#include "ConfigExpanderBase.hpp"
#include "ConfigVisualizer.hpp"

namespace fast_match {
    class FAsTMatch{
    public:
        FAsTMatch();

        virtual void init( float epsilon = 0.15f, float delta = 0.25f, bool photometric_invariance = false,
                   float min_scale = 0.5f, float max_scale = 2.0f );

        virtual void apply(cv::Mat &image, cv::Mat &templ, double &best_distance,
                              float min_rotation = static_cast<float>(-M_PI),
                              float max_rotation = static_cast<float>(M_PI));
        virtual void calculate();
        virtual std::vector<cv::Point> getBestCorners();


        bool calculateLevel();
        int no_of_points = 0;
        int level = 0;
        cv::Mat original_image;
        cv::Mat imageGray, templGray;
        float imageGrayAvg = 0.0;

    protected:
#ifdef USE_CV_GPU
        cv::cuda::GpuMat imageGrayGpu, templGrayGpu;
#endif
        cv::Mat image, templ;
        float templAvg = 0.0;
        float imageAvg = 0.0;
        float templGrayAvg = 0.0;

    public:
        virtual void setImage(const cv::Mat &image);

        virtual void setTemplate(const cv::Mat &templ);

        static std::vector<double> evaluateConfigs( cv::Mat& image, cv::Mat& templ, std::vector<cv::Mat>& affine_matrices,
                                        cv::Mat& xs, cv::Mat& ys, bool photometric_invariance );

    protected:

        cv::RNG rng;
        float epsilon;
        float delta;
        bool photometricInvariance;
        float minScale;
        float maxScale;
        cv::Size halfTempl;
        cv::Size halfImage;

        std::shared_ptr<ConfigExpanderBase> configExpander;
        ConfigVisualizer visualizer;
        bool visualize = true;



        std::vector<cv::Mat> configsToAffine( std::vector<MatchConfig>& configs, std::vector<bool>& insiders );

        std::vector<MatchConfig> getGoodConfigsByDistance( std::vector<MatchConfig>& configs, float best_dist, float new_delta,
                                                      std::vector<double>& distances, float& thresh, bool& too_high_percentage );

        float delta_fact = 1.511f;
        float new_delta;


        MatchConfig best_config;
        cv::Mat best_trans;
        std::vector<double> best_distances;
        std::vector<double> distances;
        std::vector<bool> insiders;
        std::vector<MatchConfig> configs;

    };
}

template<typename type>
std::ostream &operator <<( std::ostream& os, const std::pair<type, type> & vec ) {
    os << "[";
    os << vec.first << " " << vec.second;
    os << "]";
    return os;
}

template<typename type>
std::ostream &operator <<( std::ostream& os, const std::vector<type> & vec ) {
    os << "[";
    std::copy( vec.begin(), vec.end(), std::ostream_iterator<type>(os, ", ") );
    os << "]";
    return os;
}

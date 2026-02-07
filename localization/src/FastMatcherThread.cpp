//
// Created by rokas on 17.5.8.
//

#include <chrono>
#include <filesystem>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/lexical_cast.hpp>

#include "FastMatcherThread.hpp"

using namespace std::chrono;


FastMatcherThread::FastMatcherThread() : matcher() {
    matcher.init(0.05f, 0.9f, true, 0.9f, 1.1f);
}

void FastMatcherThread::setDirectionPrecision(double directionPrecision) {
    FastMatcherThread::directionPrecision = directionPrecision;
}


std::string gen_number(std::string directory, const int len = 5) {
    using namespace std::filesystem;
    int maxValue = 0;
    for(const auto& entry : directory_iterator(directory)) {
        std::vector<std::string> strs;
        boost::split(strs, entry.path().stem().string(), boost::is_any_of("-"));
        if(strs.size() > 1) {
            try {
                int curValue = boost::lexical_cast<int>(strs[1].c_str());
                if(curValue > maxValue) {
                    maxValue = curValue;
                }
            } catch (const boost::bad_lexical_cast& e) {
                // Don't care, just skip
            }
        }
    }
    std::string value = std::to_string(++maxValue);
    if(value.size() < len) {
        value = std::string(len - value.size(), '0') + value;
    }
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);

    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y%m%d%H%M%S");
    auto time = oss.str();
    value += "-" + time;
    return value;
}

cv::Point2f FastMatcherThread::match(cv::Mat image, cv::Mat templ, double direction) {
    double distance;
    cv::resize(image, image, cv::Size(0, 0), scaleDownFactor, scaleDownFactor, cv::INTER_NEAREST);
    cv::resize(templ, templ, cv::Size(0, 0), scaleDownFactor, scaleDownFactor, cv::INTER_NEAREST);
    high_resolution_clock::time_point t1;
    if(debug) {
        t1 = high_resolution_clock::now();
    }
    std::vector<cv::Point2f> corners = matcher.apply(
            image,
            templ,
            distance,
            static_cast<float>(-directionPrecision),
            static_cast<float>(directionPrecision)
    );
    cv::Point2f location = cv::Point2f(((corners[0].x + corners[2].x) / 2.f), ((corners[0].y + corners[2].y) / 2.f));
    if(debug) {
        cout << duration_cast<milliseconds>(high_resolution_clock::now() - t1 ).count() << ",";
        cout << (image.cols / 2.f) - location.x << "," << (image.rows / 2.f) - location.y << "\n";
        cv::Mat result;
        image.copyTo(result);
        Size sz1 = image.size();
        Size sz2 = templ.size();
        line(result, corners[0], corners[1], Scalar(0, 0, 255), 2);
        line(result, corners[1], corners[2], Scalar(0, 0, 255), 2);
        line(result, corners[2], corners[3], Scalar(0, 0, 255), 2);
        line(result, corners[3], corners[0], Scalar(0, 0, 255), 2);
        cv::Mat dest = cv::Mat::zeros(cv::Size(sz1.width + sz2.width, sz1.height), CV_8UC1);
        result.copyTo(dest.colRange(0, sz1.width).rowRange(0, sz1.height));
        templ.copyTo(dest.colRange(sz1.width, dest.cols).rowRange(0, sz2.height));
        if(debug) {
            std::string name = gen_number("/var/airvision/matching/");
            cv::imwrite("/var/airvision/matching/img-" + name + ".png", dest);
        }
    }
    return location / scaleDownFactor;
}

bool FastMatcherThread::isRunning() {
    return future_.valid() &&
           future_.wait_for(std::chrono::seconds(0)) != std::future_status::ready;
}

bool FastMatcherThread::getResultIfAvailable(cv::Point2f &result) {
    if (future_.valid() &&
        future_.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
        result = future_.get();
        return true;
    }
    return false;
}

void FastMatcherThread::matchAsync(cv::Mat image, cv::Mat templ, double direction) {
    if (!isRunning()) {
        cv::Mat im, tm;
        image.copyTo(im);
        templ.copyTo(tm);
        future_ = std::async(std::launch::async, [this, im, tm, direction] {
            return match(im, tm, direction);
        });
    }
}

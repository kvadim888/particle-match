#pragma once

#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include <fastmatch-dataset/MetadataEntry.hpp>
#include <opencv2/opencv.hpp>
#include <src/ParticleFastMatch.hpp>

class PreviewRenderer {
public:
    bool render(const MetadataEntry &metadata,
                const cv::Mat &planeView,
                std::stringstream &stringOutput,
                const cv::Mat &map,
                const cv::Point &startLocation,
                const cv::Point &svoCurPosition,
                double direction,
                const std::vector<cv::Point> &corners,
                const cv::Mat &bestTransform,
                const cv::Mat &bestView,
                const std::shared_ptr<ParticleFastMatch> &pfm);

    bool isDisplayImage() const;

    void setDisplayImage(bool displayImage);

    void setWriteImageToDisk(bool writeImageToDisk);

    void setOutputDirectory(const std::string &outputDirectory);

private:
    static void visualizeGT(const cv::Point &loc, double yaw, cv::Mat &image, int radius, int thickness,
                            const cv::Scalar &color = CV_RGB(255, 255, 0));

    void ensureWindow();

    bool writeImageToDisk = false;
    bool displayImage = true;
    std::string outputDirectory;
    bool windowInitialized = false;
    int imageCounter = 0;
};

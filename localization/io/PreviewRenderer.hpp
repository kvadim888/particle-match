#pragma once

#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include <fastmatch-dataset/MetadataEntry.hpp>
#include <opencv2/opencv.hpp>
#include <src/ParticleFastMatch.hpp>

struct RenderContext {
    const MetadataEntry &metadata;
    const cv::Mat &planeView;
    const cv::Point &startLocation;
    const cv::Point &svoCurPosition;
    double direction;
    const std::vector<cv::Point> &corners;
    const cv::Mat &bestTransform;
    const cv::Mat &bestView;
    const std::shared_ptr<ParticleFastMatch> &pfm;
};

class PreviewRenderer {
public:
    bool render(const RenderContext &ctx, std::stringstream &stringOutput);

    bool isDisplayImage() const;

    void setDisplayImage(bool displayImage);

    void setWriteImageToDisk(bool writeImageToDisk);

    void setOutputDirectory(const std::string &outputDirectory);

private:
    static void visualizeGT(const cv::Point &loc, double yaw, cv::Mat &image, int radius, int thickness,
                            const cv::Scalar &color = CV_RGB(255, 255, 0));

    void ensureWindow();

    bool writeImageToDisk_ = false;
    bool displayImage_ = true;
    std::string outputDirectory_;
    bool windowInitialized_ = false;
    int imageCounter_ = 0;
};

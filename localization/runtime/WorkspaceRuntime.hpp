#pragma once

#include "RuntimeBase.hpp"
#include "io/PreviewRenderer.hpp"

class WorkspaceRuntime : public RuntimeBase {
public:
    bool preview(const MetadataEntry &metadata, const cv::Mat &image, std::stringstream &stringOutput) override;

    bool isDisplayImage() const override;

    void setDisplayImage(bool displayImage) override;

    void setWriteImageToDisk(bool writeImageToDisk) override;

    void setOutputDirectory(const std::string &outputDirectory) override;

private:
    PreviewRenderer renderer_;
};

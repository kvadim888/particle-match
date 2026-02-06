#include "WorkspaceRuntime.hpp"

bool WorkspaceRuntime::preview(const MetadataEntry &metadata, const cv::Mat &image, std::stringstream &stringOutput) {
    return renderer_.render(
            metadata,
            image,
            stringOutput,
            metadata.map,
            startLocation_,
            svoCurPosition_,
            direction_,
            corners_,
            bestTransform_,
            bestView_,
            core_.getFilter()
    );
}

bool WorkspaceRuntime::isDisplayImage() const {
    return renderer_.isDisplayImage();
}

void WorkspaceRuntime::setDisplayImage(bool displayImage) {
    renderer_.setDisplayImage(displayImage);
}

void WorkspaceRuntime::setWriteImageToDisk(bool writeImageToDisk) {
    renderer_.setWriteImageToDisk(writeImageToDisk);
}

void WorkspaceRuntime::setOutputDirectory(const std::string &outputDirectory) {
    renderer_.setOutputDirectory(outputDirectory);
}

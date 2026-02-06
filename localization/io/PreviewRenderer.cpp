#include "PreviewRenderer.hpp"

#include <cmath>
#include <iostream>

#include <boost/filesystem/path.hpp>

#include <src/Utilities.hpp>

#include "ResultWriter.hpp"

namespace fs = boost::filesystem;

namespace {
constexpr int kViewportMarginX = 1000;
constexpr int kViewportMarginY = 1000;
constexpr int kViewportWidth = 3000;
constexpr int kViewportHeight = 2000;
constexpr int kPreviewWidth = 1200;
constexpr int kPreviewHeight = 800;
constexpr int kGtMarkerRadius = 50;
constexpr int kGtMarkerThickness = 3;
} // namespace

bool PreviewRenderer::render(const RenderContext &ctx, std::stringstream &stringOutput) {
    cv::Point2i prediction = ctx.pfm->getPredictedLocation();
    cv::Point2i relativeLocation = prediction - ctx.startLocation;
    cv::Point2i offset = cv::Point2i(
            -(prediction.x - kViewportMarginX),
            -(prediction.y - kViewportMarginY)
    );
    cv::Mat mapDisplay = ctx.metadata.map(cv::Rect(
            prediction.x - kViewportMarginX,
            prediction.y - kViewportMarginY,
            kViewportWidth,
            kViewportHeight
    )).clone();
    ctx.pfm->visualizeParticles(mapDisplay, offset);
    if(!ctx.corners.empty()) {
        std::vector<cv::Point> newCorners = {
                ctx.corners[0] + offset,
                ctx.corners[1] + offset,
                ctx.corners[2] + offset,
                ctx.corners[3] + offset
        };
        cv::line(mapDisplay, newCorners[0], newCorners[1], cv::Scalar(0, 0, 255), 4);
        cv::line(mapDisplay, newCorners[1], newCorners[2], cv::Scalar(0, 0, 255), 4);
        cv::line(mapDisplay, newCorners[2], newCorners[3], cv::Scalar(0, 0, 255), 4);
        cv::line(mapDisplay, newCorners[3], newCorners[0], cv::Scalar(0, 0, 255), 4);
        cv::Point2i arrowhead((newCorners[0].x + newCorners[1].x) / 2, (newCorners[0].y + newCorners[1].y) / 2);
        cv::Point2i center((newCorners[0].x + newCorners[2].x) / 2, (newCorners[0].y + newCorners[2].y) / 2);
        cv::arrowedLine(mapDisplay, center, arrowhead, CV_RGB(255, 0, 0), 20);
    }
    visualizeGT(ctx.metadata.mapLocation + offset, ctx.direction, mapDisplay, kGtMarkerRadius, kGtMarkerThickness, CV_RGB(255, 255, 0));
    visualizeGT(prediction + offset, ctx.direction, mapDisplay, kGtMarkerRadius, kGtMarkerThickness, CV_RGB(255, 255, 255));
    cv::Rect planeViewROI = cv::Rect(
            (mapDisplay.cols - 1) - ctx.planeView.cols,
            0,
            ctx.planeView.cols,
            ctx.planeView.rows
    );
    ctx.planeView.copyTo(mapDisplay(planeViewROI));
    cv::rectangle(mapDisplay, planeViewROI, cv::Scalar(0, 0, 255));

    int fontFace = cv::FONT_HERSHEY_COMPLEX_SMALL;
    double fontScale = 2;
    int thickness = 3;
    int textOffset = 50;

    cv::putText(mapDisplay, "Simulated view", cv::Point(planeViewROI.x + 10, planeViewROI.y + textOffset),
                fontFace, fontScale, Scalar::all(255), thickness, 8);


    if(!ctx.bestTransform.empty()) {
        std::cout << ctx.bestTransform << "\n";
        cv::Mat best = Utilities::extractWarpedMapPart(ctx.metadata.map, ctx.metadata.getImage().size(), ctx.bestTransform);
        auto bestParticleROI = cv::Rect(
                (mapDisplay.cols - 1) - best.cols,
                ctx.planeView.rows,
                best.cols,
                best.rows
        );
        best.copyTo(mapDisplay(bestParticleROI));
        cv::rectangle(mapDisplay, bestParticleROI, cv::Scalar(0, 0, 255));
        cv::putText(mapDisplay, "Best particle view",
                    cv::Point(bestParticleROI.x + 10, bestParticleROI.y + textOffset),
                    fontFace, fontScale, Scalar::all(255), thickness, 8);
    }
    if(!ctx.bestView.empty()) {
        auto bestParticleROI = cv::Rect(
                (mapDisplay.cols - 1) - ctx.bestView.cols,
                ctx.planeView.rows,
                ctx.bestView.cols,
                ctx.bestView.rows
        );
        if (ctx.bestView.channels() == 1) {
            cv::cvtColor(ctx.bestView, mapDisplay(bestParticleROI), cv::COLOR_GRAY2BGR);
        } else if (ctx.bestView.channels() == 3) {
            ctx.bestView.copyTo(mapDisplay(bestParticleROI));
        }
        cv::rectangle(mapDisplay, bestParticleROI, cv::Scalar(0, 0, 255));
        cv::putText(mapDisplay, "Best particle " + std::to_string(ctx.pfm->getParticles().back().getCorrelation()),
                    cv::Point(bestParticleROI.x + 10, bestParticleROI.y + textOffset),
                    fontFace, fontScale, Scalar::all(255), thickness, 8);
    }
    double distance = sqrt(pow(ctx.metadata.mapLocation.x - prediction.x, 2) + pow(ctx.metadata.mapLocation.y - prediction.y, 2));
    double svoDistance = sqrt(pow(ctx.metadata.mapLocation.x - ctx.svoCurPosition.x, 2) +
                                      pow(ctx.metadata.mapLocation.y - ctx.svoCurPosition.y, 2));
    ResultWriter::appendRow(
            stringOutput,
            ctx.pfm->particleCount(),
            relativeLocation,
            distance,
            svoDistance
    );
    cv::putText(mapDisplay, "Location error = " + std::to_string(distance) + " m",
                cv::Point(10, textOffset), fontFace, fontScale, Scalar::all(255), thickness, 8);
    if(writeImageToDisk_) {
        char integers[6];
        std::snprintf(integers, 6, "%05d", imageCounter_++);
        std::string filename = "preview_" + std::string(integers) + ".jpg";
        fs::path p(outputDirectory_);
        cv::imwrite((p / filename).string(), mapDisplay);
    }
    if(displayImage_) {
        ensureWindow();
        cv::Mat preview;
        cv::resize(mapDisplay, preview, cv::Size(kPreviewWidth, kPreviewHeight));
        cv::imshow("Map", preview);
        int key = cv::waitKey(10);
        return key != 27;
    }
    return true;
}

bool PreviewRenderer::isDisplayImage() const {
    return displayImage_;
}

void PreviewRenderer::setDisplayImage(bool displayImage) {
    displayImage_ = displayImage;
}

void PreviewRenderer::setWriteImageToDisk(bool writeImageToDisk) {
    writeImageToDisk_ = writeImageToDisk;
}

void PreviewRenderer::setOutputDirectory(const std::string &outputDirectory) {
    outputDirectory_ = outputDirectory;
}

void PreviewRenderer::visualizeGT(const cv::Point &loc, double yaw, cv::Mat &image, int radius, int thickness,
                                  const cv::Scalar &color) {
    cv::circle(image, loc, radius, color, thickness);
    cv::line(
            image,
            loc,
            cv::Point(
                    static_cast<int>(loc.x + (4 * radius * sin(yaw))),
                    static_cast<int>(loc.y - (4 * radius * cos(yaw)))
            ),
            color,
            thickness
    );
}

void PreviewRenderer::ensureWindow() {
    if(windowInitialized_) {
        return;
    }
    cv::namedWindow("Map", cv::WINDOW_NORMAL);
    cv::waitKey(10);
    windowInitialized_ = true;
}

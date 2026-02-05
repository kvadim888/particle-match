#include "PreviewRenderer.hpp"

#include <cmath>
#include <iostream>

#include <boost/filesystem/path.hpp>

#include <src/Utilities.hpp>

#include "ResultWriter.hpp"

namespace fs = boost::filesystem;

bool PreviewRenderer::render(const MetadataEntry &metadata,
                             const cv::Mat &planeView,
                             std::stringstream &stringOutput,
                             const cv::Mat &map,
                             const cv::Point &startLocation,
                             const cv::Point &svoCurPosition,
                             double direction,
                             const std::vector<cv::Point> &corners,
                             const cv::Mat &bestTransform,
                             const cv::Mat &bestView,
                             const std::shared_ptr<ParticleFastMatch> &pfm) {
    cv::Point2i prediction = pfm->getPredictedLocation();
    cv::Point2i relativeLocation = prediction - startLocation;
    cv::Point2i offset = cv::Point2i(
            -(prediction.x - 1000),
            -(prediction.y - 1000)
    );
    cv::Mat mapDisplay = map(cv::Rect(
            prediction.x - 1000,
            prediction.y - 1000,
            3000,
            2000
    )).clone();
    pfm->visualizeParticles(mapDisplay, offset);
    if(!corners.empty()) {
        std::vector<cv::Point> newCorners = {
                corners[0] + offset,
                corners[1] + offset,
                corners[2] + offset,
                corners[3] + offset
        };
        cv::line(mapDisplay, newCorners[0], newCorners[1], cv::Scalar(0, 0, 255), 4);
        cv::line(mapDisplay, newCorners[1], newCorners[2], cv::Scalar(0, 0, 255), 4);
        cv::line(mapDisplay, newCorners[2], newCorners[3], cv::Scalar(0, 0, 255), 4);
        cv::line(mapDisplay, newCorners[3], newCorners[0], cv::Scalar(0, 0, 255), 4);
        cv::Point2i arrowhead((newCorners[0].x + newCorners[1].x) / 2, (newCorners[0].y + newCorners[1].y) / 2);
        cv::Point2i center((newCorners[0].x + newCorners[2].x) / 2, (newCorners[0].y + newCorners[2].y) / 2);
        cv::arrowedLine(mapDisplay, center, arrowhead, CV_RGB(255, 0, 0), 20);
    }
    visualizeGT(metadata.mapLocation + offset, direction, mapDisplay, 50, 3, CV_RGB(255, 255, 0));
    visualizeGT(prediction + offset, direction, mapDisplay, 50, 3, CV_RGB(255, 255, 255));
    cv::Rect planeViewROI = cv::Rect(
            (mapDisplay.cols - 1) - planeView.cols,
            0,
            planeView.cols,
            planeView.rows
    );
    planeView.copyTo(mapDisplay(planeViewROI));
    cv::rectangle(mapDisplay, planeViewROI, cv::Scalar(0, 0, 255));

    int fontFace = cv::FONT_HERSHEY_COMPLEX_SMALL;
    double fontScale = 2;
    int thickness = 3;
    int textOffset = 50;

    cv::putText(mapDisplay, "Simulated view", cv::Point(planeViewROI.x + 10, planeViewROI.y + textOffset),
                fontFace, fontScale, Scalar::all(255), thickness, 8);


    if(!bestTransform.empty()) {
        std::cout << bestTransform << "\n";
        cv::Mat best = Utilities::extractWarpedMapPart(metadata.map, metadata.getImage().size(), bestTransform);
        auto bestParticleROI = cv::Rect(
                (mapDisplay.cols - 1) - best.cols,
                planeView.rows,
                best.cols,
                best.rows
        );
        best.copyTo(mapDisplay(bestParticleROI));
        cv::rectangle(mapDisplay, bestParticleROI, cv::Scalar(0, 0, 255));
        cv::putText(mapDisplay, "Best particle view",
                    cv::Point(bestParticleROI.x + 10, bestParticleROI.y + textOffset),
                    fontFace, fontScale, Scalar::all(255), thickness, 8);
    }
    if(!bestView.empty()) {
        auto bestParticleROI = cv::Rect(
                (mapDisplay.cols - 1) - bestView.cols,
                planeView.rows,
                bestView.cols,
                bestView.rows
        );
        if (bestView.channels() == 1) {
            cv::cvtColor(bestView, mapDisplay(bestParticleROI), cv::COLOR_GRAY2BGR);
        } else if (bestView.channels() == 3) {
            bestView.copyTo(mapDisplay(bestParticleROI));
        }
        cv::rectangle(mapDisplay, bestParticleROI, cv::Scalar(0, 0, 255));
        cv::putText(mapDisplay, "Best particle " + std::to_string(pfm->getParticles().back().getCorrelation()),
                    cv::Point(bestParticleROI.x + 10, bestParticleROI.y + textOffset),
                    fontFace, fontScale, Scalar::all(255), thickness, 8);
    }
    double distance = sqrt(pow(metadata.mapLocation.x - prediction.x, 2) + pow(metadata.mapLocation.y - prediction.y, 2));
    double svoDistance = sqrt(pow(metadata.mapLocation.x - svoCurPosition.x, 2) +
                                      pow(metadata.mapLocation.y - svoCurPosition.y, 2));
    ResultWriter::appendRow(
            stringOutput,
            pfm->particleCount(),
            relativeLocation,
            distance,
            svoDistance
    );
    cv::putText(mapDisplay, "Location error = " + std::to_string(distance) + " m",
                cv::Point(10, textOffset), fontFace, fontScale, Scalar::all(255), thickness, 8);
    if(writeImageToDisk) {
        char integers[6];
        std::snprintf(integers, 6, "%05d", imageCounter++);
        std::string filename = "preview_" + std::string(integers) + ".jpg";
        fs::path p(outputDirectory);
        cv::imwrite((p / filename).string(), mapDisplay);
    }
    if(displayImage) {
        ensureWindow();
        cv::Mat preview;
        cv::resize(mapDisplay, preview, cv::Size(1200, 800));
        cv::imshow("Map", preview);
        int key = cv::waitKey(10);
        return key != 27;
    }
    return true;
}

bool PreviewRenderer::isDisplayImage() const {
    return displayImage;
}

void PreviewRenderer::setDisplayImage(bool displayImage) {
    PreviewRenderer::displayImage = displayImage;
}

void PreviewRenderer::setWriteImageToDisk(bool writeImageToDisk) {
    PreviewRenderer::writeImageToDisk = writeImageToDisk;
}

void PreviewRenderer::setOutputDirectory(const std::string &outputDirectory) {
    PreviewRenderer::outputDirectory = outputDirectory;
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
    if(windowInitialized) {
        return;
    }
    cv::namedWindow("Map", cv::WINDOW_NORMAL);
    cv::waitKey(10);
    windowInitialized = true;
}

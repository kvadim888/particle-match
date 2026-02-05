#include "ResultWriter.hpp"

#include <iomanip>
#include <ostream>

void ResultWriter::appendHeader(std::ostream &out) {
    out << "\"ParticleCount [count]\",";
    out << "\"RelativePosX [map px]\",";
    out << "\"RelativePosY [map px]\",";
    out << "\"LocationError [map px]\",";
    out << "\"SVODistance [map px]\"";
}

void ResultWriter::appendRow(std::ostream &out,
                             uint32_t particleCount,
                             const cv::Point &relativeLocation,
                             double distance,
                             double svoDistance) {
    out << particleCount << ",";
    out << relativeLocation.x << "," << relativeLocation.y << ",";
    auto flags = out.flags();
    auto precision = out.precision();
    out << std::fixed << std::setprecision(2) << distance << "," << svoDistance;
    out.flags(flags);
    out.precision(precision);
}

#include "TestFramework.hpp"
#include "io/ResultWriter.hpp"

#include <sstream>
#include <string>

void test_header_format() {
    std::ostringstream out;
    ResultWriter::appendHeader(out);
    std::string header = out.str();

    test::check(header.find("ParticleCount") != std::string::npos, "header contains ParticleCount");
    test::check(header.find("RelativePosX") != std::string::npos, "header contains RelativePosX");
    test::check(header.find("RelativePosY") != std::string::npos, "header contains RelativePosY");
    test::check(header.find("LocationError") != std::string::npos, "header contains LocationError");
    test::check(header.find("SVODistance") != std::string::npos, "header contains SVODistance");
}

void test_row_format() {
    std::ostringstream out;
    ResultWriter::appendRow(out, 150, cv::Point(10, 20), 5.678, 12.345);
    std::string row = out.str();

    test::check(row.find("150") != std::string::npos, "row contains particle count");
    test::check(row.find("10") != std::string::npos, "row contains x coordinate");
    test::check(row.find("20") != std::string::npos, "row contains y coordinate");
    test::check(row.find("5.68") != std::string::npos, "row contains formatted distance");
    test::check(row.find("12.35") != std::string::npos ||
                row.find("12.34") != std::string::npos, "row contains formatted svo distance");
}

void test_row_preserves_stream_state() {
    std::ostringstream out;
    out << std::scientific;
    auto flags_before = out.flags();
    auto prec_before = out.precision();

    ResultWriter::appendRow(out, 100, cv::Point(0, 0), 1.0, 2.0);

    test::check(out.flags() == flags_before, "stream flags preserved after appendRow");
    test::check(out.precision() == prec_before, "stream precision preserved after appendRow");
}

int main() {
    std::cout << "=== ResultWriter Tests ===\n";
    test_header_format();
    test_row_format();
    test_row_preserves_stream_state();
    return test::report();
}

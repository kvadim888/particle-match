#include "TestFramework.hpp"
#include "models/ScaleModel.hpp"

#include <cmath>

void test_scale_calculation() {
    ScaleModel model;
    float capturedMin = 0, capturedMax = 0;
    float result = model.updateScale(1.0f, 100.0f, 640,
        [&](float min, float max) { capturedMin = min; capturedMax = max; });

    float expected = (std::tan(0.5f) * 100.0f) / 320.0f;
    test::check_near(result, expected, 0.001, "scale calculation correct");
    test::check_near(capturedMin, expected * ScaleModel::kScaleMarginLow, 0.001, "min scale uses kScaleMarginLow");
    test::check_near(capturedMax, expected * ScaleModel::kScaleMarginHigh, 0.001, "max scale uses kScaleMarginHigh");
}

void test_scale_proportional_to_altitude() {
    ScaleModel model;
    float scale100 = 0, scale200 = 0;
    auto noop = [](float, float) {};

    scale100 = model.updateScale(1.0f, 100.0f, 640, noop);
    scale200 = model.updateScale(1.0f, 200.0f, 640, noop);

    test::check_near(scale200 / scale100, 2.0, 0.001, "scale doubles when altitude doubles");
}

void test_scale_inversely_proportional_to_width() {
    ScaleModel model;
    float scale640 = 0, scale1280 = 0;
    auto noop = [](float, float) {};

    scale640 = model.updateScale(1.0f, 100.0f, 640, noop);
    scale1280 = model.updateScale(1.0f, 100.0f, 1280, noop);

    test::check_near(scale640 / scale1280, 2.0, 0.001, "scale halves when width doubles");
}

void test_margin_constants() {
    test::check(ScaleModel::kScaleMarginLow < 1.0f, "kScaleMarginLow < 1.0");
    test::check(ScaleModel::kScaleMarginHigh > 1.0f, "kScaleMarginHigh > 1.0");
    test::check(ScaleModel::kScaleMarginLow > 0.0f, "kScaleMarginLow > 0.0");
}

int main() {
    std::cout << "=== ScaleModel Tests ===\n";
    test_scale_calculation();
    test_scale_proportional_to_altitude();
    test_scale_inversely_proportional_to_width();
    test_margin_constants();
    return test::report();
}

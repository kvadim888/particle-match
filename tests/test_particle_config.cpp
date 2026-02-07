#include "TestFramework.hpp"
#include "src/ParticleConfig.hpp"

void test_default_r_initial_count() {
    ParticleConfig config;
    test::check(config.r_initial.size() == 13, "r_initial has 13 elements");
}

void test_r_initial_symmetry() {
    ParticleConfig config;
    // r_initial should be symmetric around 0
    for (size_t i = 0; i < config.r_initial.size() / 2; ++i) {
        size_t mirror = config.r_initial.size() - 1 - i;
        test::check_near(config.r_initial[i], -config.r_initial[mirror], 1e-6,
                         "r_initial[" + std::to_string(i) + "] mirrors r_initial[" + std::to_string(mirror) + "]");
    }
}

void test_r_initial_center_is_zero() {
    ParticleConfig config;
    test::check_near(config.r_initial[6], 0.0, 1e-6, "r_initial center element is 0");
}

void test_default_r_step() {
    ParticleConfig config;
    test::check_near(config.r_step, 0.05f, 1e-6, "default r_step is 0.05");
}

void test_default_direction() {
    ParticleConfig config;
    test::check_near(config.direction, 0.0, 1e-6, "default direction is 0");
}

void test_set_map_dimensions() {
    ParticleConfig config;
    config.setMapDimensions(cv::Size(800, 600));
    test::check(config.mapCenter.x == 400, "mapCenter.x is width/2");
    test::check(config.mapCenter.y == 300, "mapCenter.y is height/2");
}

void test_set_map_dimensions_odd() {
    ParticleConfig config;
    config.setMapDimensions(cv::Size(801, 601));
    test::check(config.mapCenter.x == 400, "mapCenter.x integer division for odd width");
    test::check(config.mapCenter.y == 300, "mapCenter.y integer division for odd height");
}

void test_r_initial_range() {
    ParticleConfig config;
    float expected_min = -(3 * config.r_step);
    float expected_max = 3 * config.r_step;
    test::check_near(config.r_initial.front(), expected_min, 1e-6, "r_initial first = -3*step");
    test::check_near(config.r_initial.back(), expected_max, 1e-6, "r_initial last = +3*step");
}

int main() {
    std::cout << "=== ParticleConfig Tests ===\n";
    test_default_r_initial_count();
    test_r_initial_symmetry();
    test_r_initial_center_is_zero();
    test_default_r_step();
    test_default_direction();
    test_set_map_dimensions();
    test_set_map_dimensions_odd();
    test_r_initial_range();
    return test::report();
}

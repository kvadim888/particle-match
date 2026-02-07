#include "TestFramework.hpp"
#include "core/ParticleFilterConfig.hpp"

void test_default_config_valid() {
    ParticleFilterConfig config;
    test::check_nothrow([&]{ config.validate(); }, "default config passes validation");
}

void test_negative_radius() {
    ParticleFilterConfig config;
    config.radius = -1.0;
    test::check_throws([&]{ config.validate(); }, "negative radius throws");
}

void test_zero_radius() {
    ParticleFilterConfig config;
    config.radius = 0.0;
    test::check_throws([&]{ config.validate(); }, "zero radius throws");
}

void test_epsilon_out_of_range() {
    ParticleFilterConfig config;

    config.epsilon = 0.0f;
    test::check_throws([&]{ config.validate(); }, "zero epsilon throws");

    config.epsilon = 1.0f;
    test::check_throws([&]{ config.validate(); }, "epsilon=1.0 throws");

    config.epsilon = -0.1f;
    test::check_throws([&]{ config.validate(); }, "negative epsilon throws");
}

void test_particle_count_zero() {
    ParticleFilterConfig config;
    config.particleCount = 0;
    test::check_throws([&]{ config.validate(); }, "zero particleCount throws");
}

void test_quantile_out_of_range() {
    ParticleFilterConfig config;

    config.quantile = 0.0f;
    test::check_throws([&]{ config.validate(); }, "zero quantile throws");

    config.quantile = 1.5f;
    test::check_throws([&]{ config.validate(); }, "quantile > 1 throws");

    config.quantile = 1.0f;
    test::check_nothrow([&]{ config.validate(); }, "quantile=1.0 is valid");
}

void test_kld_error_zero() {
    ParticleFilterConfig config;
    config.kld_error = 0.0f;
    test::check_throws([&]{ config.validate(); }, "zero kld_error throws");
}

void test_bin_size_zero() {
    ParticleFilterConfig config;
    config.binSize = 0;
    test::check_throws([&]{ config.validate(); }, "zero binSize throws");
}

void test_valid_custom_config() {
    ParticleFilterConfig config;
    config.radius = 100.0;
    config.epsilon = 0.5f;
    config.particleCount = 50;
    config.quantile = 0.5f;
    config.kld_error = 1.0f;
    config.binSize = 10;
    config.use_gaussian = false;
    test::check_nothrow([&]{ config.validate(); }, "custom valid config passes");
}

int main() {
    std::cout << "=== ParticleFilterConfig Tests ===\n";
    test_default_config_valid();
    test_negative_radius();
    test_zero_radius();
    test_epsilon_out_of_range();
    test_particle_count_zero();
    test_quantile_out_of_range();
    test_kld_error_zero();
    test_bin_size_zero();
    test_valid_custom_config();
    return test::report();
}

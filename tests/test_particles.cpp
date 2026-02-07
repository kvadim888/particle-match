#include "TestFramework.hpp"
#include "src/Particle.hpp"
#include "src/ParticleConfig.hpp"

#include <memory>
#include <cmath>

namespace {
std::shared_ptr<ParticleConfig> makeConfig() {
    auto cfg = std::make_shared<ParticleConfig>();
    cfg->setMapDimensions(cv::Size(4000, 3000));
    return cfg;
}
} // namespace

void test_particle_serialize_basic() {
    auto cfg = makeConfig();
    Particle p(100, 200, cfg);
    std::string s = p.serialize(5);
    // binSize=5: 100/5=20, 200/5=40 -> "20x40"
    test::check(s == "20x40", "serialize(5) produces binned string: got " + s);
}

void test_particle_serialize_bin_size_1() {
    auto cfg = makeConfig();
    Particle p(7, 13, cfg);
    std::string s = p.serialize(1);
    test::check(s == "7x13", "serialize(1) produces exact coords: got " + s);
}

void test_particle_serialize_bin_rounding() {
    auto cfg = makeConfig();
    Particle p(12, 17, cfg);
    std::string s = p.serialize(10);
    // 12/10=1, 17/10=1 -> "1x1"
    test::check(s == "1x1", "serialize(10) truncates: got " + s);
}

void test_particle_probability_moving_average() {
    auto cfg = makeConfig();
    Particle p(100, 100, cfg);

    p.setProbability(1.0f);
    // After 1 iteration: avg = 1.0/1 = 1.0
    test::check_near(p.getProbability(), 1.0, 0.01, "probability after 1 set");

    p.setProbability(0.0f);
    // After 2 iterations: avg = (1.0+0.0)/2 = 0.5
    test::check_near(p.getProbability(), 0.5, 0.01, "probability after 2 sets (moving avg)");
}

void test_particle_weight_and_sampling() {
    auto cfg = makeConfig();
    Particle p(50, 50, cfg);
    p.setWeight(0.75f);
    test::check_near(p.getWeight(), 0.75f, 1e-5, "weight getter/setter");

    p.setSamplingFactor(0.25f);
    test::check_near(p.getSamplingFactor(), 0.25f, 1e-5, "samplingFactor getter/setter");
}

void test_particle_ordering() {
    auto cfg = makeConfig();
    Particle a(0, 0, cfg);
    Particle b(0, 0, cfg);
    a.setSamplingFactor(0.3f);
    b.setSamplingFactor(0.7f);
    test::check(a < b, "particle with lower samplingFactor is less");
    test::check(b > a, "particle with higher samplingFactor is greater");
}

void test_particle_copy() {
    auto cfg = makeConfig();
    Particle orig(42, 84, cfg);
    orig.setProbability(0.6f);
    orig.setWeight(0.3f);

    Particle copy(orig);
    test::check(copy.x == 42 && copy.y == 84, "copy preserves coordinates");
    test::check_near(copy.getWeight(), 0.3f, 1e-5, "copy preserves weight");
}

int main() {
    std::cout << "=== Particle & Serialize Tests ===\n";
    test_particle_serialize_basic();
    test_particle_serialize_bin_size_1();
    test_particle_serialize_bin_rounding();
    test_particle_probability_moving_average();
    test_particle_weight_and_sampling();
    test_particle_ordering();
    test_particle_copy();
    return test::report();
}

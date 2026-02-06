#pragma once

#include <cmath>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace test {

struct TestResult {
    std::string name;
    bool passed;
    std::string message;
};

inline std::vector<TestResult>& results() {
    static std::vector<TestResult> r;
    return r;
}

inline void check(bool condition, const std::string& name, const std::string& detail = "") {
    results().push_back({name, condition, detail});
    if (!condition) {
        std::cerr << "  FAIL: " << name;
        if (!detail.empty()) std::cerr << " -- " << detail;
        std::cerr << "\n";
    }
}

inline void check_throws(std::function<void()> fn, const std::string& name) {
    bool threw = false;
    try { fn(); } catch (...) { threw = true; }
    check(threw, name, threw ? "" : "expected exception but none was thrown");
}

inline void check_nothrow(std::function<void()> fn, const std::string& name) {
    bool threw = false;
    std::string what;
    try { fn(); } catch (const std::exception& e) { threw = true; what = e.what(); } catch (...) { threw = true; what = "unknown"; }
    check(!threw, name, threw ? "unexpected exception: " + what : "");
}

inline void check_near(double a, double b, double eps, const std::string& name) {
    bool ok = std::fabs(a - b) < eps;
    std::ostringstream ss;
    if (!ok) ss << a << " vs " << b << " (eps=" << eps << ")";
    check(ok, name, ss.str());
}

inline int report() {
    int passed = 0, failed = 0;
    for (const auto& r : results()) {
        if (r.passed) ++passed; else ++failed;
    }
    std::cout << "\n" << passed << " passed, " << failed << " failed, " << (passed + failed) << " total\n";
    return failed > 0 ? 1 : 0;
}

} // namespace test

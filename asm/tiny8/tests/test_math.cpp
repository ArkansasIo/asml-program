#include "tiny8/math.hpp"

#include <cassert>
#include <cmath>
#include <iostream>

int main() {
    using namespace tiny8::math;

    assert(add(250, 10) == 4);       // 8-bit wraparound
    assert(sub(3, 5) == 254);
    assert(mul(20, 13) == 4);
    assert(!div(12, 0).has_value());
    assert(div(21, 3).value() == 7);
    assert(isqrt(80) == 8);

    assert(solve_linear(3, 2, 14).value() == 4);
    assert(!solve_linear(0, 2, 14).has_value());
    assert(!solve_linear(3, 0, 14).has_value()); // not an integer solution

    const auto qsum = qadd(q4_4::from_int(2), q4_4::from_int(3));
    assert(std::abs(qsum.to_double() - 5.0) < 0.001);

    const auto qproduct = qmul(q4_4::from_int(2), q4_4::from_int(3));
    assert(std::abs(qproduct.to_double() - 6.0) < 0.001);

    const auto roots = solve_quadratic(1.0, -3.0, 2.0);
    assert(roots.has_value());
    assert(std::abs(roots->first - 1.0) < 1e-9);
    assert(std::abs(roots->second - 2.0) < 1e-9);

    const double slope = derivative([](double x) { return x * x; }, 3.0);
    assert(std::abs(slope - 6.0) < 0.01);

    const double area = integrate([](double x) { return x; }, 0.0, 4.0, 1000);
    assert(std::abs(area - 8.0) < 0.001);

    std::cout << "Tiny8 math tests passed\n";
}

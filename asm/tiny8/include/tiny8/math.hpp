#pragma once
// Tiny8 math library: 8-bit arithmetic, algebra, and fixed-point numerical calculus.
// This is a host-side companion library for the Tiny8 emulator, not native Tiny8
// machine code. Values are deliberately constrained to unsigned 8-bit integers
// or signed Q4.4 fixed-point values where documented.

#include <array>
#include <cstdint>
#include <optional>
#include <span>

namespace tiny8::math {

using u8 = std::uint8_t;
using i8 = std::int8_t;

// Basic 8-bit arithmetic. Results wrap modulo 256, matching Tiny8 registers.
[[nodiscard]] constexpr u8 add(u8 a, u8 b) noexcept {
    return static_cast<u8>(static_cast<unsigned>(a) + b);
}
[[nodiscard]] constexpr u8 sub(u8 a, u8 b) noexcept {
    return static_cast<u8>(static_cast<unsigned>(a) - b);
}
[[nodiscard]] constexpr u8 mul(u8 a, u8 b) noexcept {
    return static_cast<u8>(static_cast<unsigned>(a) * b);
}
[[nodiscard]] constexpr std::optional<u8> div(u8 a, u8 b) noexcept {
    if (b == 0) return std::nullopt;
    return static_cast<u8>(a / b);
}
[[nodiscard]] constexpr u8 bit_and(u8 a, u8 b) noexcept { return static_cast<u8>(a & b); }
[[nodiscard]] constexpr u8 bit_or(u8 a, u8 b) noexcept { return static_cast<u8>(a | b); }
[[nodiscard]] constexpr u8 bit_xor(u8 a, u8 b) noexcept { return static_cast<u8>(a ^ b); }
[[nodiscard]] constexpr u8 bit_not(u8 a) noexcept { return static_cast<u8>(~a); }
[[nodiscard]] constexpr u8 min(u8 a, u8 b) noexcept { return a < b ? a : b; }
[[nodiscard]] constexpr u8 max(u8 a, u8 b) noexcept { return a > b ? a : b; }
[[nodiscard]] constexpr u8 abs_diff(u8 a, u8 b) noexcept {
    return a > b ? static_cast<u8>(a - b) : static_cast<u8>(b - a);
}

// Solve a*x + b = c using integer arithmetic. No solution is returned when
// a == 0 or the solution is not an integer in the unsigned 8-bit range.
[[nodiscard]] constexpr std::optional<u8> solve_linear(u8 a, u8 b, u8 c) noexcept {
    if (a == 0 || c < b) return std::nullopt;
    const unsigned numerator = static_cast<unsigned>(c) - b;
    if (numerator % a != 0) return std::nullopt;
    const unsigned x = numerator / a;
    if (x > 255) return std::nullopt;
    return static_cast<u8>(x);
}

// Unsigned integer square root: floor(sqrt(n)).
[[nodiscard]] constexpr u8 isqrt(u8 n) noexcept {
    unsigned result = 0;
    while ((result + 1) * (result + 1) <= n) ++result;
    return static_cast<u8>(result);
}

// Signed Q4.4 fixed point: raw value / 16. Range is -8.0 through 7.9375.
// Arithmetic is saturating rather than wrapping to reduce numerical surprises.
struct q4_4 {
    i8 raw{0};
    [[nodiscard]] static constexpr q4_4 from_raw(i8 value) noexcept { return {value}; }
    [[nodiscard]] static constexpr q4_4 from_int(int value) noexcept {
        const int scaled = value * 16;
        return {static_cast<i8>(scaled < -128 ? -128 : scaled > 127 ? 127 : scaled)};
    }
    [[nodiscard]] constexpr double to_double() const noexcept {
        return static_cast<double>(raw) / 16.0;
    }
};

[[nodiscard]] constexpr q4_4 qadd(q4_4 a, q4_4 b) noexcept {
    const int v = static_cast<int>(a.raw) + static_cast<int>(b.raw);
    return {static_cast<i8>(v < -128 ? -128 : v > 127 ? 127 : v)};
}
[[nodiscard]] constexpr q4_4 qsub(q4_4 a, q4_4 b) noexcept {
    const int v = static_cast<int>(a.raw) - static_cast<int>(b.raw);
    return {static_cast<i8>(v < -128 ? -128 : v > 127 ? 127 : v)};
}
[[nodiscard]] constexpr q4_4 qmul(q4_4 a, q4_4 b) noexcept {
    const int v = (static_cast<int>(a.raw) * static_cast<int>(b.raw)) / 16;
    return {static_cast<i8>(v < -128 ? -128 : v > 127 ? 127 : v)};
}
[[nodiscard]] constexpr std::optional<q4_4> qdiv(q4_4 a, q4_4 b) noexcept {
    if (b.raw == 0) return std::nullopt;
    const int v = (static_cast<int>(a.raw) * 16) / static_cast<int>(b.raw);
    return q4_4{static_cast<i8>(v < -128 ? -128 : v > 127 ? 127 : v)};
}

// Polynomial coefficients are ordered from constant term upwards:
// p(x) = coefficients[0] + coefficients[1]*x + ...
// Horner evaluation is performed in Q4.4 fixed point.
[[nodiscard]] constexpr q4_4 evaluate_polynomial(
    std::span<const q4_4> coefficients, q4_4 x) noexcept {
    q4_4 result{};
    for (std::size_t i = coefficients.size(); i > 0; --i) {
        result = qadd(qmul(result, x), coefficients[i - 1]);
    }
    return result;
}

// Numerical derivative using the central difference:
// f'(x) ~= (f(x+h)-f(x-h))/(2h). Uses double precision on the host.
template <class Function>
[[nodiscard]] double derivative(Function&& f, double x, double h = 1.0 / 256.0) {
    if (h <= 0.0) return 0.0;
    return (f(x + h) - f(x - h)) / (2.0 * h);
}

// Composite trapezoidal integral. Returns 0 for invalid intervals or steps.
// This numerical integration routine runs on the host, not on the 8-bit ISA.
template <class Function>
[[nodiscard]] double integrate(Function&& f, double a, double b,
                               unsigned steps = 128) {
    if (steps == 0 || a == b) return 0.0;
    const double h = (b - a) / static_cast<double>(steps);
    double sum = 0.5 * (f(a) + f(b));
    for (unsigned i = 1; i < steps; ++i) {
        sum += f(a + h * static_cast<double>(i));
    }
    return sum * h;
}

// Solve ax^2 + bx + c = 0 over real numbers. Returns no roots for a==0,
// negative discriminant, or unsupported degenerate equations.
struct quadratic_roots {
    double first{};
    double second{};
};
[[nodiscard]] inline std::optional<quadratic_roots>
solve_quadratic(double a, double b, double c) noexcept {
    if (a == 0.0) return std::nullopt;
    const double discriminant = b * b - 4.0 * a * c;
    if (discriminant < 0.0) return std::nullopt;
    const double root = __builtin_sqrt(discriminant);
    return quadratic_roots{(-b - root) / (2.0 * a),
                           (-b + root) / (2.0 * a)};
}

} // namespace tiny8::math

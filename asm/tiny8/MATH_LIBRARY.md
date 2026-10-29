# Tiny8 Math Library

A companion C++23 header library for the Tiny8 8-bit CPU emulator. It provides 8-bit integer arithmetic, elementary algebra, Q4.4 fixed-point operations, polynomial evaluation, and host-side numerical calculus.

> Important: the C++ templates for derivatives and integrals run on the host computer. They are not native Tiny8 instructions. The Tiny8 CPU currently has a small instruction set; these are tools for programs and experiments built around the emulator.

## Header

Include `asm/tiny8/include/tiny8/math.hpp`.

## Features

- **8-bit arithmetic:** add, subtract, multiply (all modulo 256), division with explicit divide-by-zero handling, bitwise operations, min/max, and absolute difference.
- **Algebra:** solve `a*x + b = c` when the solution is an integer in the unsigned 8-bit range; unsigned integer square root.
- **Fixed point:** signed Q4.4 values (scale 16, range -8 to 7.9375), saturating addition/subtraction/multiplication/division.
- **Polynomials:** Horner evaluation of coefficients ordered from constant term upward.
- **Calculus:** central-difference numerical derivative and composite trapezoidal numerical integration.
- **Equations:** real roots of a quadratic equation when its leading coefficient is nonzero and its discriminant is nonnegative.

## Build and run tests

From the repository root, with a C++23 compiler:

```sh
c++ -std=c++23 -Wall -Wextra -Wpedantic \
  -I asm/tiny8/include asm/tiny8/tests/test_math.cpp -o tiny8_math_tests
./tiny8_math_tests
```

Expected output:

```text
Tiny8 math tests passed
```

## Examples

```cpp
#include "tiny8/math.hpp"
using namespace tiny8::math;

auto sum = add(250, 10);                 // 4: 8-bit wraparound
auto x = solve_linear(3, 2, 14);         // x = 4
auto roots = solve_quadratic(1, -3, 2);  // roots 1 and 2
double slope = derivative([](double v) { return v*v; }, 3.0); // about 6
double area = integrate([](double v) { return v; }, 0, 4);    // about 8
```

## Numeric caveats

- 8-bit integer operations wrap modulo 256 by design.
- Q4.4 fixed-point values have limited range and precision; arithmetic saturates.
- Numerical derivatives and integrals are approximations; accuracy depends on the step size and function behavior.
- Quadratic solving uses floating-point math and is host-side, not an 8-bit integer-only algorithm.
- This library is educational and not suitable for safety-critical or physical machine control.

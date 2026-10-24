#pragma once
#include "lcs/Types.hpp"
namespace lcs::simulation {
class Engine { public: explicit Engine(double rateHz=20.0); bool initialize(); bool tick(double dtSeconds); const lcs::core::Environment& environment() const noexcept; const lcs::core::Position& position() const noexcept; double rateHz() const noexcept; private: double rateHz_; lcs::core::Environment environment_{}; lcs::core::Position position_{}; };
}

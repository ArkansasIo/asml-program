#pragma once
#include "lcs/Types.hpp"
namespace lcs::stage {
struct Limits { double xMin{-100.0}; double xMax{100.0}; double yMin{-100.0}; double yMax{100.0}; double zMin{-10.0}; double zMax{10.0}; };
class Model {
public:
 explicit Model(Limits limits = {}): limits_(limits) {}
 bool moveTo(lcs::core::Position target) noexcept;
 bool moveBy(lcs::core::Position delta) noexcept;
 const lcs::core::Position& position() const noexcept { return position_; }
 const Limits& limits() const noexcept { return limits_; }
private:
 bool within(lcs::core::Position p) const noexcept;
 Limits limits_{};
 lcs::core::Position position_{};
};
}

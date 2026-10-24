#include "lcs/Types.hpp"
#include <cmath>
namespace lcs::core {
bool isFinite(double value) noexcept { return std::isfinite(value); }
bool isValidPosition(const Position& value) noexcept { return isFinite(value.x)&&isFinite(value.y)&&isFinite(value.z); }
bool isValidEnvironment(const Environment& value) noexcept { return isFinite(value.temperatureK)&&isFinite(value.pressurePa)&&value.temperatureK>0.0&&value.pressurePa>=0.0; }
}

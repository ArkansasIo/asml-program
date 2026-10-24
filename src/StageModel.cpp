#include "lcs/StageModel.hpp"
#include <cmath>
namespace lcs::stage {
bool Model::within(lcs::core::Position p) const noexcept { return std::isfinite(p.x)&&std::isfinite(p.y)&&std::isfinite(p.z)&&p.x>=limits_.xMin&&p.x<=limits_.xMax&&p.y>=limits_.yMin&&p.y<=limits_.yMax&&p.z>=limits_.zMin&&p.z<=limits_.zMax; }
bool Model::moveTo(lcs::core::Position target) noexcept { if(!within(target)) return false; position_=target; return true; }
bool Model::moveBy(lcs::core::Position delta) noexcept { return moveTo({position_.x+delta.x,position_.y+delta.y,position_.z+delta.z}); }
}

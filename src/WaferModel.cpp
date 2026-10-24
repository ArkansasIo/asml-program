#include "lcs/WaferModel.hpp"
#include <cmath>
namespace lcs::wafer {
bool Model::valid() const noexcept { return std::isfinite(spec_.diameterMm)&&spec_.diameterMm>0.0&&std::isfinite(spec_.edgeExclusionMm)&&spec_.edgeExclusionMm>=0.0&&spec_.edgeExclusionMm<spec_.diameterMm/2.0&&spec_.rows>0&&spec_.columns>0; }
Site Model::site(std::size_t row,std::size_t column) const noexcept { const double usable=spec_.diameterMm-2.0*spec_.edgeExclusionMm; const double sx=spec_.columns>1?usable/static_cast<double>(spec_.columns-1):0.0; const double sy=spec_.rows>1?usable/static_cast<double>(spec_.rows-1):0.0; return {row,column,-usable/2.0+static_cast<double>(column)*sx,-usable/2.0+static_cast<double>(row)*sy}; }
}

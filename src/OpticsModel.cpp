#include "lcs/OpticsModel.hpp"
namespace lcs::optics {
double Model::illustrativeResolutionNm() const noexcept { if(!valid()) return 0.0; return 0.61*parameters_.wavelengthNm/parameters_.numericalAperture; }
}

#pragma once
#include <cmath>
namespace lcs::optics {
struct Parameters { double wavelengthNm{193.0}; double numericalAperture{0.93}; double reduction{4.0}; };
class Model {
public:
 explicit Model(Parameters p = {}): parameters_(p) {}
 bool valid() const noexcept { return std::isfinite(parameters_.wavelengthNm)&&parameters_.wavelengthNm>0.0&&std::isfinite(parameters_.numericalAperture)&&parameters_.numericalAperture>0.0&&parameters_.numericalAperture<=2.0&&std::isfinite(parameters_.reduction)&&parameters_.reduction>=1.0; }
 double illustrativeResolutionNm() const noexcept;
 const Parameters& parameters() const noexcept { return parameters_; }
private: Parameters parameters_{};
};
}

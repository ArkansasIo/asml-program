#pragma once
#include <cstddef>
namespace lcs::wafer {
struct Specification { double diameterMm{300.0}; double edgeExclusionMm{3.0}; std::size_t rows{10}; std::size_t columns{10}; };
struct Site { std::size_t row{}; std::size_t column{}; double xMm{}; double yMm{}; };
class Model {
public:
 explicit Model(Specification spec = {}): spec_(spec) {}
 bool valid() const noexcept;
 Site site(std::size_t row, std::size_t column) const noexcept;
 const Specification& specification() const noexcept { return spec_; }
private: Specification spec_{};
};
}

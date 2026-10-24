#include "lcs/SafetyInterlock.hpp"
namespace lcs::safety {
void Interlock::require(std::string name,bool satisfied){ if(!satisfied) failures_.push_back(std::move(name)); }
bool Interlock::allSatisfied() const noexcept { return failures_.empty(); }
}

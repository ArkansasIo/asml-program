#include "lcs/JobModel.hpp"
#include <cmath>
#include <utility>
namespace lcs::job {
bool Model::addExposure(Exposure e){ if(state_!=State::Draft||e.id.empty()||!std::isfinite(e.xMm)||!std::isfinite(e.yMm)||!std::isfinite(e.energy)||!std::isfinite(e.focus)||e.energy<0.0) return false; exposures_.push_back(std::move(e)); return true; }
bool Model::validate(std::string& error) const { if(id_.empty()){error="job id is empty";return false;} if(exposures_.empty()){error="job contains no exposures";return false;} error.clear(); return true; }
}

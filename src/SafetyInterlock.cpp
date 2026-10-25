#include "lcs/SafetyInterlock.hpp"
#include <algorithm>
namespace lcs::safety {
void Interlock::require(std::string name,bool satisfied){
    if(satisfied){
        auto it=std::remove(failures_.begin(),failures_.end(),name);
        failures_.erase(it,failures_.end());
    } else if(std::find(failures_.begin(),failures_.end(),name)==failures_.end()){
        failures_.push_back(std::move(name));
    }
}
bool Interlock::allSatisfied() const noexcept { return failures_.empty(); }
}

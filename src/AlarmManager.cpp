#include "lcs/AlarmManager.hpp"
#include <algorithm>
#include <utility>
namespace lcs::alarms {
void Manager::raise(std::string code,lcs::core::Severity severity,std::string message){auto it=std::find_if(alarms_.begin(),alarms_.end(),[&](const auto&a){return a.code==code;});if(it!=alarms_.end()){it->severity=severity;it->message=std::move(message);it->active=true;return;}alarms_.push_back({nextId_++,severity,std::move(code),std::move(message),true});}
void Manager::clear(const std::string& code){auto it=std::find_if(alarms_.begin(),alarms_.end(),[&](const auto&a){return a.code==code;});if(it!=alarms_.end())it->active=false;}
bool Manager::hasCritical() const noexcept{return std::any_of(alarms_.begin(),alarms_.end(),[](const auto&a){return a.active&&a.severity==lcs::core::Severity::Critical;});}
const std::vector<lcs::core::Alarm>& Manager::active() const noexcept{return alarms_;}
}

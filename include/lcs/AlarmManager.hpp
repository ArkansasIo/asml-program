#pragma once
#include "lcs/Types.hpp"
#include <vector>
namespace lcs::alarms {
class Manager { public: void raise(std::string code,lcs::core::Severity severity,std::string message); void clear(const std::string& code); bool hasCritical() const noexcept; const std::vector<lcs::core::Alarm>& active() const noexcept; private: std::vector<lcs::core::Alarm> alarms_; std::uint64_t nextId_{1}; };
}

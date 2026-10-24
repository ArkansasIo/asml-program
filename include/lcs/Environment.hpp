#pragma once
#include <string>
namespace lcs::config {
class EnvironmentLoader { public: static std::string get(const char* name,std::string fallback={}); static bool getBool(const char* name,bool fallback=false); };
}

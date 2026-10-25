#include "lcs/Environment.hpp"
#include <cstdlib>
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4996) // getenv is the only portable way here
#endif
namespace lcs::config {
std::string EnvironmentLoader::get(const char* name,std::string fallback){if(const char* value=std::getenv(name);value)return value;return fallback;}
bool EnvironmentLoader::getBool(const char* name,bool fallback){const auto value=get(name);if(value.empty())return fallback;return value=="1"||value=="true"||value=="TRUE"||value=="yes";}
#if defined(_MSC_VER)
#pragma warning(pop)
#endif
}

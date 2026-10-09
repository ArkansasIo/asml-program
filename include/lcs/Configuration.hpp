#pragma once
#include <string>
#include <unordered_map>
namespace lcs::config {
class Configuration { std::unordered_map<std::string,std::string> values_;
public: void set(std::string,std::string); std::string get(const std::string&,const std::string& fallback={}) const; };
}

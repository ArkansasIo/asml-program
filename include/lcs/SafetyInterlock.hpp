#pragma once
#include <string>
#include <vector>
namespace lcs::safety {
class Interlock {
public:
 void require(std::string name, bool satisfied);
 bool allSatisfied() const noexcept;
 const std::vector<std::string>& failures() const noexcept { return failures_; }
 void clear() { failures_.clear(); }
private: std::vector<std::string> failures_;
};
}

#pragma once
#include "lcs/Types.hpp"
#include <string>
#include <utility>
#include <vector>
namespace lcs::job {
enum class State { Draft, Validated, Queued, Running, Completed, Failed, Cancelled };
struct Exposure { std::string id; double xMm{}; double yMm{}; double energy{}; double focus{}; };
class Model {
public:
 explicit Model(std::string id = "SIM-JOB"): id_(std::move(id)) {}
 bool addExposure(Exposure e);
 bool validate(std::string& error) const;
 void start() noexcept { state_=State::Running; }
 void complete() noexcept { state_=State::Completed; }
 State state() const noexcept { return state_; }
 const std::vector<Exposure>& exposures() const noexcept { return exposures_; }
 const std::string& id() const noexcept { return id_; }
private:
 std::string id_; State state_{State::Draft}; std::vector<Exposure> exposures_;
};
}

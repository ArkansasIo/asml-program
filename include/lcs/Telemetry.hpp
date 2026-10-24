#pragma once
#include <chrono>
#include <cstdint>
#include <string>
#include <vector>
namespace lcs::telemetry {
using Clock=std::chrono::system_clock;
struct Sample { std::uint64_t sensorId{}; double value{}; std::string unit; Clock::time_point recordedAt{Clock::now()}; };
class Buffer { public: explicit Buffer(std::size_t capacity=4096); bool push(Sample sample); std::vector<Sample> drain(); std::size_t size() const noexcept; private: std::size_t capacity_; std::vector<Sample> samples_; };
}

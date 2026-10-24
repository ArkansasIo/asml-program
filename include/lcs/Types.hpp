#pragma once
#include <cstdint>
#include <string>
namespace lcs::core {
struct Position { double x{}; double y{}; double z{}; };
struct Environment { double temperatureK{293.15}; double pressurePa{101325.0}; };
enum class MachineState : std::uint8_t { Offline, Ready, Running, Paused, Fault };
enum class Severity : std::uint8_t { Info, Warning, Critical };
struct Alarm { std::uint64_t id{}; Severity severity{Severity::Info}; std::string code; std::string message; bool active{false}; };
bool isFinite(double value) noexcept;
bool isValidPosition(const Position&) noexcept;
bool isValidEnvironment(const Environment&) noexcept;
}

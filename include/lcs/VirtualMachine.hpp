#pragma once
#include "lcs/Math.hpp"
namespace lcs::simulation {
enum class State { Offline,Ready,Running,Paused,Fault };
struct Snapshot { State state{State::Offline}; lcs::math::Vec3 position{}; double temperatureK{293.15}; double pressurePa{101325.0}; };
class VirtualMachine {
 Snapshot snapshot_{};
public:
 void initialize(); void start(); void pause(); void stop(); void move(double,double,double);
 const Snapshot& snapshot() const { return snapshot_; }
};
}

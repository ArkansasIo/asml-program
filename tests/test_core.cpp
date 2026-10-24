#include <cassert>
#include <cmath>
#include "lcs/AlarmManager.hpp"
#include "lcs/Math.hpp"
#include "lcs/SimulationEngine.hpp"
#include "lcs/Telemetry.hpp"
#include "lcs/Types.hpp"
#include "lcs/VirtualMachine.hpp"

void runSimulationModelTests();

int main(){
 assert(std::abs(lcs::math::magnitude({3,4,0})-5.0)<1e-9);
 assert(lcs::math::clamp(5.0,0.0,4.0)==4.0);
 assert(lcs::math::lerp(0.0,10.0,0.25)==2.5);
 lcs::simulation::VirtualMachine vm; vm.initialize(); assert(vm.snapshot().state==lcs::simulation::State::Ready); vm.start(); assert(vm.snapshot().state==lcs::simulation::State::Running); vm.pause(); assert(vm.snapshot().state==lcs::simulation::State::Paused); vm.stop(); assert(vm.snapshot().state==lcs::simulation::State::Ready);
 lcs::simulation::Engine sim(20.0); assert(sim.initialize()); assert(sim.tick(0.05)); assert(!sim.tick(-1.0));
 lcs::telemetry::Buffer buffer(2); assert(buffer.push({1,10.0,"Pa"})); assert(buffer.push({1,11.0,"Pa"})); assert(!buffer.push({1,12.0,"Pa"})); assert(buffer.drain().size()==2);
 lcs::alarms::Manager alarms; alarms.raise("SIM.TEST",lcs::core::Severity::Critical,"Test alarm"); assert(alarms.hasCritical()); alarms.clear("SIM.TEST"); assert(!alarms.hasCritical());
 assert(lcs::core::isValidEnvironment({293.15,101325.0})); assert(!lcs::core::isValidEnvironment({-1.0,101325.0}));
 runSimulationModelTests();
 return 0;
}

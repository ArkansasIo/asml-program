#include <cassert>
#include <cmath>
#include "lcs/Math.hpp"
#include "lcs/VirtualMachine.hpp"
int main(){assert(std::abs(lcs::math::magnitude({3,4,0})-5.0)<1e-9);lcs::simulation::VirtualMachine vm;vm.initialize();vm.start();assert(vm.snapshot().state==lcs::simulation::State::Running);vm.pause();assert(vm.snapshot().state==lcs::simulation::State::Paused);return 0;}

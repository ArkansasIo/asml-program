// LCS desktop shell placeholder.
// Console shell that drives the virtual (simulation-only) backend.
// Native GUI remains future work; physical control is disabled.
#include <iostream>

#include "lcs/VirtualMachine.hpp"

int main()
{
    lcs::simulation::VirtualMachine vm;
    vm.initialize();
    vm.start();

    const auto &s = vm.snapshot();
    std::cout << "Lithography Control Studio desktop shell\n";
    std::cout << "Backend: virtual simulator (physical control disabled)\n";
    std::cout << "State: " << static_cast<int>(s.state) << "\n";
    std::cout << "Position: (" << s.position.x << ", " << s.position.y << ", " << s.position.z << ")\n";
    std::cout << "Temperature: " << s.temperatureK << " K, Pressure: " << s.pressurePa << " Pa\n";
    return 0;
}

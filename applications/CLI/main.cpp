// LCS command-line simulation shell.
// Interactive control of the virtual (simulation-only) machine backend.
// Physical equipment control is intentionally not implemented.
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "lcs/VirtualMachine.hpp"
#include "lcs/Math.hpp"

namespace {

void printHelp(std::ostream& out) {
    out << "Commands:\n"
        << "  help                 show this help\n"
        << "  init                 initialize the virtual machine\n"
        << "  start                start the simulation\n"
        << "  pause                pause the simulation\n"
        << "  stop                 stop the simulation\n"
        << "  move <x> <y> <z>     move stage to position (mm)\n"
        << "  status               print machine snapshot\n"
        << "  mag <x> <y> <z>      vector magnitude via lcs::math\n"
        << "  quit                 exit the shell\n";
}

void printSnapshot(std::ostream& out, const lcs::simulation::Snapshot& s) {
    static const char* names[] = {"Offline", "Ready", "Running", "Paused", "Fault"};
    auto idx = static_cast<int>(s.state);
    out << "state=" << (idx >= 0 && idx < 5 ? names[idx] : "Unknown")
        << " position=(" << s.position.x << ", " << s.position.y << ", " << s.position.z << ")"
        << " temperatureK=" << s.temperatureK
        << " pressurePa=" << s.pressurePa << '\n';
}

}  // namespace

int main() {
    lcs::simulation::VirtualMachine vm;
    std::cout << "LCS C++23 CLI shell (simulation backend, physical control disabled)\n";
    printHelp(std::cout);

    std::string line;
    while (true) {
        std::cout << "lcs> " << std::flush;
        if (!std::getline(std::cin, line)) break;

        std::istringstream iss(line);
        std::string cmd;
        if (!(iss >> cmd)) continue;

        if (cmd == "quit" || cmd == "exit") break;
        if (cmd == "help") { printHelp(std::cout); continue; }
        if (cmd == "init") { vm.initialize(); printSnapshot(std::cout, vm.snapshot()); continue; }
        if (cmd == "start") { vm.start(); printSnapshot(std::cout, vm.snapshot()); continue; }
        if (cmd == "pause") { vm.pause(); printSnapshot(std::cout, vm.snapshot()); continue; }
        if (cmd == "stop") { vm.stop(); printSnapshot(std::cout, vm.snapshot()); continue; }
        if (cmd == "status") { printSnapshot(std::cout, vm.snapshot()); continue; }

        if (cmd == "move" || cmd == "mag") {
            double x = 0, y = 0, z = 0;
            if (!(iss >> x >> y >> z)) {
                std::cout << "usage: " << cmd << " <x> <y> <z>\n";
                continue;
            }
            if (cmd == "move") {
                vm.move(x, y, z);
                printSnapshot(std::cout, vm.snapshot());
            } else {
                std::cout << "magnitude=" << lcs::math::magnitude({x, y, z}) << '\n';
            }
            continue;
        }

        std::cout << "unknown command: " << cmd << " (try 'help')\n";
    }
    return 0;
}

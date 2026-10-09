#include <iostream>
#include "lcs/VirtualMachine.hpp"
int main(){lcs::simulation::VirtualMachine vm;vm.initialize();std::cout<<"Lithography Control Studio desktop shell\\n";std::cout<<"Virtual backend state="<<static_cast<int>(vm.snapshot().state)<<"\\n";return 0;}

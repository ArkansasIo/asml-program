#include "lcs/VirtualMachine.hpp"
namespace lcs::simulation {
void VirtualMachine::initialize(){snapshot_.state=State::Ready;}
void VirtualMachine::start(){if(snapshot_.state==State::Ready||snapshot_.state==State::Paused)snapshot_.state=State::Running;}
void VirtualMachine::pause(){if(snapshot_.state==State::Running)snapshot_.state=State::Paused;}
void VirtualMachine::stop(){snapshot_.state=State::Ready;}
void VirtualMachine::move(double x,double y,double z){snapshot_.position={x,y,z};}
}

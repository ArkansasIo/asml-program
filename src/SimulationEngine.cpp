#include "lcs/SimulationEngine.hpp"
#include <cmath>
#include <stdexcept>
namespace lcs::simulation {
Engine::Engine(double rateHz):rateHz_(rateHz){if(!std::isfinite(rateHz)||rateHz<=0.0)throw std::invalid_argument("simulation rate must be positive");}
bool Engine::initialize(){environment_={};position_={};return true;}
bool Engine::tick(double dtSeconds){return std::isfinite(dtSeconds)&&dtSeconds>=0.0&&dtSeconds<=10.0;}
const lcs::core::Environment& Engine::environment() const noexcept{return environment_;}
const lcs::core::Position& Engine::position() const noexcept{return position_;}
double Engine::rateHz() const noexcept{return rateHz_;}
}

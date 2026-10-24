#include <cassert>
#include <cmath>
#include "lcs/JobModel.hpp"
#include "lcs/OpticsModel.hpp"
#include "lcs/SafetyInterlock.hpp"
#include "lcs/StageModel.hpp"
#include "lcs/Units.hpp"
#include "lcs/WaferModel.hpp"

void runSimulationModelTests(){
    assert(lcs::units::toMillimeters(1.0,lcs::units::Length::Meter)==1000.0);
    assert(lcs::units::toNanometers(1.0,lcs::units::Length::Micrometer)==1000.0);
    lcs::stage::Model stage; assert(stage.moveTo({1.0,2.0,3.0})); assert(!stage.moveTo({1000.0,0.0,0.0}));
    lcs::wafer::Model wafer; assert(wafer.valid()); auto s=wafer.site(0,0); assert(std::isfinite(s.xMm)&&std::isfinite(s.yMm));
    lcs::optics::Model optics; assert(optics.valid()); assert(optics.illustrativeResolutionNm()>0.0);
    lcs::job::Model job("SIM-001"); assert(job.addExposure({"E1",0.0,0.0,10.0,0.0})); std::string error; assert(job.validate(error));
    lcs::safety::Interlock interlock; interlock.require("simulation",true); interlock.require("operator",false); assert(!interlock.allSatisfied());
}

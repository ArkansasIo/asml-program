// Control-plane unit tests: state gating, interlocks, limits, lifecycle.
#include <cassert>
#include <iostream>

#include "lcs/ControlPlanes.hpp"

using namespace lcs::control;

static void testMachinePlaneLifecycle()
{
    ControlPlanes cp;
    auto &m = cp.machine();
    assert(m.initialize());
    assert(m.snapshot().state == lcs::simulation::State::Ready);
    assert(m.start());
    assert(m.snapshot().state == lcs::simulation::State::Running);
    assert(m.pause());
    assert(m.snapshot().state == lcs::simulation::State::Paused);
    assert(m.stop());
    assert(m.snapshot().state == lcs::simulation::State::Ready);
}

static void testSafetyGateBlocksAllPlanes()
{
    ControlPlanes cp;
    cp.safety().update("enclosure_doors_closed", false); // open a door
    assert(!cp.safety().allSatisfied());
    assert(!cp.machine().initialize());                     // gated
    assert(!cp.machine().start());                          // gated
    assert(!cp.stage().moveTo({1, 2, 3}));                  // gated
    assert(!cp.optics().setParameters({193.0, 0.93, 4.0})); // gated
    // Restore safety: commands flow again.
    cp.safety().update("enclosure_doors_closed", true);
    assert(cp.safety().allSatisfied());
    assert(cp.machine().initialize());
    assert(cp.machine().start());
    assert(cp.stage().moveTo({1, 2, 3}));
}

static void testStageLimits()
{
    ControlPlanes cp;
    assert(cp.stage().moveTo({50, 50, 5}));  // inside limits
    assert(!cp.stage().moveTo({500, 0, 0})); // outside x limit
    assert(!cp.stage().moveTo({0, 0, 50}));  // outside z limit
    assert(cp.stage().moveBy({10, 0, 0}));   // relative, stays inside
    const auto &p = cp.stage().position();
    assert(p.x == 10.0 && p.y == 0.0 && p.z == 0.0);
}

static void testOpticsValidation()
{
    ControlPlanes cp;
    assert(cp.optics().setParameters({193.0, 0.93, 4.0}));
    assert(!cp.optics().setParameters({0.0, 0.93, 4.0}));   // bad wavelength
    assert(!cp.optics().setParameters({193.0, 5.0, 4.0}));  // impossible NA
    assert(!cp.optics().setParameters({193.0, 0.93, 0.5})); // reduction < 1
    assert(cp.optics().resolutionNm() > 0.0);
}

static void testAlarmLifecycle()
{
    ControlPlanes cp;
    cp.alarms().raise("VAC-01", lcs::core::Severity::Warning, "vacuum drift");
    assert(cp.alarms().active().size() == 1);
    assert(!cp.alarms().hasCritical());
    cp.alarms().raise("STG-99", lcs::core::Severity::Critical, "stage fault");
    assert(cp.alarms().hasCritical());
    assert(cp.alarms().clear("VAC-01"));
    assert(!cp.alarms().clear("VAC-01")); // already cleared
    assert(cp.alarms().active().size() == 1);
    assert(cp.alarms().clear("STG-99"));
    assert(!cp.alarms().hasCritical());
}

static void testTelemetryBuffering()
{
    ControlPlanes cp;
    assert(cp.telemetry().push({1, 10.0, "Pa"}));
    assert(cp.telemetry().push({1, 11.0, "Pa"}));
    assert(cp.telemetry().size() == 2);
    auto drained = cp.telemetry().drain();
    assert(drained.size() == 2);
    assert(cp.telemetry().size() == 0);
}

static void testJobLifecycle()
{
    ControlPlanes cp;
    std::string error;
    assert(cp.jobs().create("JOB-1", error));
    lcs::job::Exposure e{"E1", 1.0, 2.0, 10.0, -0.1};
    assert(cp.jobs().addExposure("JOB-1", e, error));
    assert(cp.jobs().validate("JOB-1", error));
    assert(cp.jobs().queue("JOB-1", error));
    assert(cp.jobs().start("JOB-1", error));
    assert(cp.jobs().complete("JOB-1", error));
    // Invalid flows must be refused.
    assert(!cp.jobs().start("NOPE", error));
    assert(!cp.jobs().complete("NOPE", error));
    assert(!cp.jobs().create("JOB-1", error)); // duplicate
    assert(!error.empty());
}

static void testAuditTrailRecordsCommands()
{
    ControlPlanes cp;
    cp.machine().initialize();
    cp.stage().moveTo({5, 5, 0});
    const auto &entries = cp.audit().entries();
    assert(entries.size() >= 2);
    bool sawMachine = false, sawStage = false;
    for (const auto &e : entries)
    {
        sawMachine = sawMachine || e.plane == "machine";
        sawStage = sawStage || e.plane == "stage";
    }
    assert(sawMachine && sawStage);
}

static void testConfigPlane()
{
    ControlPlanes cp;
    cp.config().set("exposure.defaultEnergy", "12.5");
    assert(cp.config().get("exposure.defaultEnergy") == "12.5");
    assert(cp.config().get("missing.key", "fallback") == "fallback");
}

int main()
{
    testMachinePlaneLifecycle();
    testSafetyGateBlocksAllPlanes();
    testStageLimits();
    testOpticsValidation();
    testAlarmLifecycle();
    testTelemetryBuffering();
    testJobLifecycle();
    testAuditTrailRecordsCommands();
    testConfigPlane();
    std::cout << "all control-plane tests passed\n";
    return 0;
}

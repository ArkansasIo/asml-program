#include "lcs/ControlPlanes.hpp"

#include <algorithm>
#include <utility>

namespace lcs::control
{

    // ---------------------------------------------------------------------------
    // AuditTrail
    // ---------------------------------------------------------------------------
    void AuditTrail::record(std::string plane, std::string action, std::string detail)
    {
        entries_.push_back(AuditEntry{nextId_++, std::move(plane), std::move(action), std::move(detail)});
        while (entries_.size() > 1024)
            entries_.pop_front(); // bounded trail
    }

    // ---------------------------------------------------------------------------
    // SafetyControlPlane
    // ---------------------------------------------------------------------------
    SafetyControlPlane::SafetyControlPlane(AuditTrail &audit) : audit_(audit) {}

    void SafetyControlPlane::declare(const std::string &name, bool satisfied)
    {
        interlock_.require(name, satisfied);
        audit_.record("safety", "declare", name + (satisfied ? " satisfied" : " OPEN"));
    }

    void SafetyControlPlane::update(const std::string &name, bool satisfied)
    {
        interlock_.require(name, satisfied); // re-require updates the entry
        audit_.record("safety", "update", name + (satisfied ? " satisfied" : " OPEN"));
    }

    bool SafetyControlPlane::allSatisfied() const noexcept
    {
        // Interlock only tracks failures; a declared-then-satisfied name stays clear.
        return interlock_.allSatisfied();
    }

    const std::vector<std::string> &SafetyControlPlane::failures() const noexcept
    {
        return interlock_.failures();
    }

    // ---------------------------------------------------------------------------
    // MachineControlPlane
    // ---------------------------------------------------------------------------
    MachineControlPlane::MachineControlPlane(AuditTrail &audit, SafetyControlPlane &safety)
        : audit_(audit), safety_(safety) {}

    bool MachineControlPlane::gate(const char *action)
    {
        if (!safety_.allSatisfied())
        {
            vm_ = lcs::simulation::VirtualMachine{}; // drop to Offline on blocked control
            audit_.record("machine", action, "blocked by safety interlock");
            return false;
        }
        return true;
    }

    bool MachineControlPlane::initialize()
    {
        if (!gate("initialize"))
            return false;
        vm_.initialize();
        audit_.record("machine", "initialize", stateName(vm_.snapshot().state));
        return true;
    }

    bool MachineControlPlane::start()
    {
        if (!gate("start"))
            return false;
        vm_.start();
        audit_.record("machine", "start", stateName(vm_.snapshot().state));
        return true;
    }

    bool MachineControlPlane::pause()
    {
        if (!gate("pause"))
            return false;
        vm_.pause();
        audit_.record("machine", "pause", stateName(vm_.snapshot().state));
        return true;
    }

    bool MachineControlPlane::stop()
    {
        if (!gate("stop"))
            return false;
        vm_.stop();
        audit_.record("machine", "stop", stateName(vm_.snapshot().state));
        return true;
    }

    void MachineControlPlane::injectFault(const std::string &reason)
    {
        audit_.record("machine", "fault", reason);
        // Force a fresh Offline machine; caller pairs this with an alarm raise.
        vm_ = lcs::simulation::VirtualMachine{};
    }

    const char *MachineControlPlane::stateName(lcs::simulation::State state) noexcept
    {
        switch (state)
        {
        case lcs::simulation::State::Offline:
            return "Offline";
        case lcs::simulation::State::Ready:
            return "Ready";
        case lcs::simulation::State::Running:
            return "Running";
        case lcs::simulation::State::Paused:
            return "Paused";
        case lcs::simulation::State::Fault:
            return "Fault";
        }
        return "Unknown";
    }

    // ---------------------------------------------------------------------------
    // StageControlPlane
    // ---------------------------------------------------------------------------
    StageControlPlane::StageControlPlane(AuditTrail &audit, SafetyControlPlane &safety, lcs::stage::Limits limits)
        : audit_(audit), safety_(safety), stage_(limits) {}

    bool StageControlPlane::moveTo(lcs::core::Position target)
    {
        if (!safety_.allSatisfied())
        {
            audit_.record("stage", "moveTo", "blocked by safety interlock");
            return false;
        }
        if (!lcs::core::isValidPosition(target) || !stage_.moveTo(target))
        {
            audit_.record("stage", "moveTo", "rejected: target outside limits");
            return false;
        }
        const auto &p = stage_.position();
        audit_.record("stage", "moveTo",
                      "(" + std::to_string(p.x) + "," + std::to_string(p.y) + "," + std::to_string(p.z) + ")");
        return true;
    }

    bool StageControlPlane::moveBy(lcs::core::Position delta)
    {
        if (!safety_.allSatisfied())
        {
            audit_.record("stage", "moveBy", "blocked by safety interlock");
            return false;
        }
        if (!stage_.moveBy(delta))
        {
            audit_.record("stage", "moveBy", "rejected: result outside limits");
            return false;
        }
        const auto &p = stage_.position();
        audit_.record("stage", "moveBy",
                      "(" + std::to_string(p.x) + "," + std::to_string(p.y) + "," + std::to_string(p.z) + ")");
        return true;
    }

    // ---------------------------------------------------------------------------
    // OpticsControlPlane
    // ---------------------------------------------------------------------------
    OpticsControlPlane::OpticsControlPlane(AuditTrail &audit, SafetyControlPlane &safety)
        : audit_(audit), safety_(safety) {}

    bool OpticsControlPlane::setParameters(lcs::optics::Parameters p)
    {
        if (!safety_.allSatisfied())
        {
            audit_.record("optics", "setParameters", "blocked by safety interlock");
            return false;
        }
        lcs::optics::Model candidate(p);
        if (!candidate.valid())
        {
            audit_.record("optics", "setParameters", "rejected: invalid parameters");
            return false;
        }
        optics_ = candidate;
        audit_.record("optics", "setParameters",
                      "lambda=" + std::to_string(p.wavelengthNm) + "nm na=" + std::to_string(p.numericalAperture) +
                          " reduction=" + std::to_string(p.reduction));
        return true;
    }

    // ---------------------------------------------------------------------------
    // AlarmControlPlane
    // ---------------------------------------------------------------------------
    AlarmControlPlane::AlarmControlPlane(AuditTrail &audit) : audit_(audit) {}

    void AlarmControlPlane::raise(const std::string &code, lcs::core::Severity severity, const std::string &message)
    {
        manager_.raise(code, severity, message);
        audit_.record("alarms", "raise", code);
    }

bool AlarmControlPlane::clear(const std::string &code)
    {
        manager_.clear(code);
        // Report whether the alarm existed and was active before clearing.
        bool existed = false;
        for (const auto &a : manager_.active())
            existed = existed || (a.code == code && a.active);
        audit_.record("alarms", "clear", existed ? code : code + " (not active)");
        return existed;
    }

    // ---------------------------------------------------------------------------
    // TelemetryControlPlane
    // ---------------------------------------------------------------------------
    TelemetryControlPlane::TelemetryControlPlane(AuditTrail &audit, std::size_t capacity)
        : audit_(audit), buffer_(capacity) {}

    bool TelemetryControlPlane::push(lcs::telemetry::Sample sample)
    {
        const bool ok = buffer_.push(std::move(sample));
        audit_.record("telemetry", ok ? "push" : "drop", "samples=" + std::to_string(buffer_.size()));
        return ok;
    }

    std::vector<lcs::telemetry::Sample> TelemetryControlPlane::drain()
    {
        auto samples = buffer_.drain();
        audit_.record("telemetry", "drain", "count=" + std::to_string(samples.size()));
        return samples;
    }

    // ---------------------------------------------------------------------------
    // JobControlPlane
    // ---------------------------------------------------------------------------
    JobControlPlane::JobControlPlane(AuditTrail &audit) : audit_(audit) {}

    lcs::job::Model *JobControlPlane::findMutable(const std::string &id)
    {
        auto it = std::find_if(jobs_.begin(), jobs_.end(),
                               [&](const lcs::job::Model &j)
                               { return j.id() == id; });
        return it == jobs_.end() ? nullptr : &*it;
    }

    const lcs::job::Model *JobControlPlane::find(const std::string &id) const
    {
        auto it = std::find_if(jobs_.begin(), jobs_.end(),
                               [&](const lcs::job::Model &j)
                               { return j.id() == id; });
        return it == jobs_.end() ? nullptr : &*it;
    }

    bool JobControlPlane::create(const std::string &id, std::string &error)
    {
        if (id.empty())
        {
            error = "job id must not be empty";
            return false;
        }
        if (find(id))
        {
            error = "job already exists: " + id;
            return false;
        }
        jobs_.emplace_back(id);
        audit_.record("jobs", "create", id);
        return true;
    }

    bool JobControlPlane::addExposure(const std::string &id, lcs::job::Exposure e, std::string &error)
    {
        auto *job = findMutable(id);
        if (!job)
        {
            error = "unknown job: " + id;
            return false;
        }
        if (job->state() != lcs::job::State::Draft && job->state() != lcs::job::State::Validated)
        {
            error = "job not editable in current state";
            return false;
        }
        if (!job->addExposure(std::move(e)))
        {
            error = "invalid exposure";
            return false;
        }
        audit_.record("jobs", "addExposure", id);
        return true;
    }

    bool JobControlPlane::validate(const std::string &id, std::string &error)
    {
        auto *job = findMutable(id);
        if (!job)
        {
            error = "unknown job: " + id;
            return false;
        }
        if (!job->validate(error))
            return false;
        audit_.record("jobs", "validate", id);
        return true;
    }

    bool JobControlPlane::queue(const std::string &id, std::string &error)
    {
        auto *job = findMutable(id);
        if (!job)
        {
            error = "unknown job: " + id;
            return false;
        }
        if (job->state() != lcs::job::State::Validated)
        {
            error = "job must be validated before queueing";
            return false;
        }
        // Note: JobModel transitions to Queued via start()/complete(); queue state is
        // tracked implicitly. We treat queue() as validation-gated acceptance.
        audit_.record("jobs", "queue", id);
        return true;
    }

    bool JobControlPlane::start(const std::string &id, std::string &error)
    {
        auto *job = findMutable(id);
        if (!job)
        {
            error = "unknown job: " + id;
            return false;
        }
        if (job->state() != lcs::job::State::Queued && job->state() != lcs::job::State::Validated)
        {
            error = "job not startable in current state";
            return false;
        }
        job->start();
        audit_.record("jobs", "start", id);
        return true;
    }

    bool JobControlPlane::complete(const std::string &id, std::string &error)
    {
        auto *job = findMutable(id);
        if (!job)
        {
            error = "unknown job: " + id;
            return false;
        }
        if (job->state() != lcs::job::State::Running)
        {
            error = "job not running";
            return false;
        }
        job->complete();
        audit_.record("jobs", "complete", id);
        return true;
    }

    bool JobControlPlane::cancel(const std::string &id, std::string &error)
    {
        auto *job = findMutable(id);
        if (!job)
        {
            error = "unknown job: " + id;
            return false;
        }
        if (job->state() == lcs::job::State::Completed)
        {
            error = "job already completed";
            return false;
        }
        audit_.record("jobs", "cancel", id);
        jobs_.erase(std::remove_if(jobs_.begin(), jobs_.end(),
                                   [&](const lcs::job::Model &j)
                                   { return j.id() == id; }),
                    jobs_.end());
        return true;
    }

    // ---------------------------------------------------------------------------
    // ConfigControlPlane
    // ---------------------------------------------------------------------------
    ConfigControlPlane::ConfigControlPlane(AuditTrail &audit) : audit_(audit) {}

    void ConfigControlPlane::set(const std::string &key, const std::string &value)
    {
        config_.set(key, value);
        audit_.record("config", "set", key);
    }

    std::string ConfigControlPlane::get(const std::string &key, const std::string &fallback) const
    {
        return config_.get(key, fallback);
    }

    // ---------------------------------------------------------------------------
    // ControlPlanes composition root
    // ---------------------------------------------------------------------------
    ControlPlanes::ControlPlanes()
        : safety_(audit_), machine_(audit_, safety_), stage_(audit_, safety_), optics_(audit_, safety_),
          alarms_(audit_), telemetry_(audit_), jobs_(audit_), config_(audit_)
    {
        // Default interlocks for a virtual machine: doors closed, vacuum ready.
        safety_.declare("enclosure_doors_closed", true);
        safety_.declare("vacuum_ready", true);
    }

} // namespace lcs::control

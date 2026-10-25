// LCS control planes — a layered façade over the simulation models.
//
// Planes (highest level first):
//   MachineControlPlane   run/state management over VirtualMachine
//   StageControlPlane     position command dispatch with limit interlock
//   OpticsControlPlane    optical parameter validation & readback
//   SafetyControlPlane    interlock registry and gating of commands
//   AlarmControlPlane     alarm raise/clear/latch
//   TelemetryControlPlane ring-buffer sample capture
//   JobControlPlane       exposure job lifecycle (validate → queue → run → done)
//   ConfigControlPlane    key/value configuration with validation hooks
//
// Each plane validates inputs, records an audit trail of accepted commands,
// and refuses commands when the safety plane reports unsatisfied interlocks.
// All control is simulation-only; physical actuation is not implemented.
#pragma once
#include "lcs/AlarmManager.hpp"
#include "lcs/Configuration.hpp"
#include "lcs/JobModel.hpp"
#include "lcs/OpticsModel.hpp"
#include "lcs/SafetyInterlock.hpp"
#include "lcs/StageModel.hpp"
#include "lcs/Telemetry.hpp"
#include "lcs/Types.hpp"
#include "lcs/VirtualMachine.hpp"

#include <cstdint>
#include <deque>
#include <string>
#include <vector>

namespace lcs::control
{

    // ---------------------------------------------------------------------------
    // Shared audit record: every accepted plane command appends one entry.
    // ---------------------------------------------------------------------------
    struct AuditEntry
    {
        std::uint64_t id{};
        std::string plane;
        std::string action;
        std::string detail;
    };

    class AuditTrail
    {
    public:
        void record(std::string plane, std::string action, std::string detail);
        const std::deque<AuditEntry> &entries() const noexcept { return entries_; }
        std::size_t size() const noexcept { return entries_.size(); }
        void clear() noexcept
        {
            entries_.clear();
            nextId_ = 1;
        }

    private:
        std::deque<AuditEntry> entries_;
        std::uint64_t nextId_{1};
    };

    // ---------------------------------------------------------------------------
    // SafetyControlPlane — interlock registry; other planes gate on it.
    // ---------------------------------------------------------------------------
    class SafetyControlPlane
    {
    public:
        explicit SafetyControlPlane(AuditTrail &audit);

        void declare(const std::string &name, bool satisfied);
        void update(const std::string &name, bool satisfied);
        bool allSatisfied() const noexcept;
        const std::vector<std::string> &failures() const noexcept;

    private:
        AuditTrail &audit_;
        lcs::safety::Interlock interlock_;
    };

    // ---------------------------------------------------------------------------
    // MachineControlPlane — lifecycle of the virtual machine.
    // ---------------------------------------------------------------------------
    class MachineControlPlane
    {
    public:
        MachineControlPlane(AuditTrail &audit, SafetyControlPlane &safety);

        bool initialize();
        bool start();
        bool pause();
        bool stop();
        void injectFault(const std::string &reason); // simulation-only hook
        const lcs::simulation::Snapshot &snapshot() const noexcept { return vm_.snapshot(); }
        static const char *stateName(lcs::simulation::State state) noexcept;

    private:
        bool gate(const char *action); // returns false with fault state if blocked
        AuditTrail &audit_;
        SafetyControlPlane &safety_;
        lcs::simulation::VirtualMachine vm_;
    };

    // ---------------------------------------------------------------------------
    // StageControlPlane — absolute/relative moves with soft-limit interlock.
    // ---------------------------------------------------------------------------
    class StageControlPlane
    {
    public:
        StageControlPlane(AuditTrail &audit, SafetyControlPlane &safety, lcs::stage::Limits limits = {});

        bool moveTo(lcs::core::Position target);
        bool moveBy(lcs::core::Position delta);
        const lcs::core::Position &position() const noexcept { return stage_.position(); }
        const lcs::stage::Limits &limits() const noexcept { return stage_.limits(); }

    private:
        AuditTrail &audit_;
        SafetyControlPlane &safety_;
        lcs::stage::Model stage_;
    };

    // ---------------------------------------------------------------------------
    // OpticsControlPlane — parameter set/validate with readback.
    // ---------------------------------------------------------------------------
    class OpticsControlPlane
    {
    public:
        OpticsControlPlane(AuditTrail &audit, SafetyControlPlane &safety);

        bool setParameters(lcs::optics::Parameters p);
        const lcs::optics::Parameters &parameters() const noexcept { return optics_.parameters(); }
        double resolutionNm() const noexcept { return optics_.illustrativeResolutionNm(); }

    private:
        AuditTrail &audit_;
        SafetyControlPlane &safety_;
        lcs::optics::Model optics_;
    };

    // ---------------------------------------------------------------------------
    // AlarmControlPlane — raise/clear with critical latching.
    // ---------------------------------------------------------------------------
    class AlarmControlPlane
    {
    public:
        explicit AlarmControlPlane(AuditTrail &audit);

        void raise(const std::string &code, lcs::core::Severity severity, const std::string &message);
        bool clear(const std::string &code);
        bool hasCritical() const noexcept { return manager_.hasCritical(); }
        const std::vector<lcs::core::Alarm> &active() const noexcept { return manager_.active(); }

    private:
        AuditTrail &audit_;
        lcs::alarms::Manager manager_;
    };

    // ---------------------------------------------------------------------------
    // TelemetryControlPlane — bounded sample capture with drain.
    // ---------------------------------------------------------------------------
    class TelemetryControlPlane
    {
    public:
        explicit TelemetryControlPlane(AuditTrail &audit, std::size_t capacity = 4096);

        bool push(lcs::telemetry::Sample sample);
        std::vector<lcs::telemetry::Sample> drain();
        std::size_t size() const noexcept { return buffer_.size(); }

    private:
        AuditTrail &audit_;
        lcs::telemetry::Buffer buffer_;
    };

    // ---------------------------------------------------------------------------
    // JobControlPlane — exposure job lifecycle.
    // ---------------------------------------------------------------------------
    class JobControlPlane
    {
    public:
        explicit JobControlPlane(AuditTrail &audit);

        bool create(const std::string &id, std::string &error);
        bool addExposure(const std::string &id, lcs::job::Exposure e, std::string &error);
        bool validate(const std::string &id, std::string &error);
        bool queue(const std::string &id, std::string &error);
        bool start(const std::string &id, std::string &error);
        bool complete(const std::string &id, std::string &error);
        bool cancel(const std::string &id, std::string &error);
        const lcs::job::Model *find(const std::string &id) const;

    private:
        lcs::job::Model *findMutable(const std::string &id);
        AuditTrail &audit_;
        std::vector<lcs::job::Model> jobs_;
    };

    // ---------------------------------------------------------------------------
    // ConfigControlPlane — typed key/value configuration.
    // ---------------------------------------------------------------------------
    class ConfigControlPlane
    {
    public:
        explicit ConfigControlPlane(AuditTrail &audit);

        void set(const std::string &key, const std::string &value);
        std::string get(const std::string &key, const std::string &fallback = {}) const;

    private:
        AuditTrail &audit_;
        lcs::config::Configuration config_;
    };

    // ---------------------------------------------------------------------------
    // ControlPlanes — composition root owning every plane.
    // ---------------------------------------------------------------------------
    class ControlPlanes
    {
    public:
        ControlPlanes();

        AuditTrail &audit() noexcept { return audit_; }
        SafetyControlPlane &safety() noexcept { return safety_; }
        MachineControlPlane &machine() noexcept { return machine_; }
        StageControlPlane &stage() noexcept { return stage_; }
        OpticsControlPlane &optics() noexcept { return optics_; }
        AlarmControlPlane &alarms() noexcept { return alarms_; }
        TelemetryControlPlane &telemetry() noexcept { return telemetry_; }
        JobControlPlane &jobs() noexcept { return jobs_; }
        ConfigControlPlane &config() noexcept { return config_; }

    private:
        AuditTrail audit_;
        SafetyControlPlane safety_;
        MachineControlPlane machine_;
        StageControlPlane stage_;
        OpticsControlPlane optics_;
        AlarmControlPlane alarms_;
        TelemetryControlPlane telemetry_;
        JobControlPlane jobs_;
        ConfigControlPlane config_;
    };

} // namespace lcs::control

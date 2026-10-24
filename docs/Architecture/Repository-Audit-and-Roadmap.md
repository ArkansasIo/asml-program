# ASML Program — Repository Audit and Roadmap

## Overview

Repository: [ArkansasIo/asml-program](https://github.com/ArkansasIo/asml-program)

Project name: Lithography Control Studio  
Version: 0.1.0 foundation  
Language: C++23  
Build system: CMake 3.25+ / Ninja  
Primary target: Windows

This repository is an early engineering and simulation foundation. It is not a production ASML equipment controller. Simulation is the default. Any future physical equipment integration must use authorized vendor interfaces, formal risk review, and equipment-native interlocks.

## Current Structure

- `applications/CLI`: command-line entry point
- `applications/Desktop`: current desktop shell entry point
- `include/lcs`: core public headers
- `src`: mathematics, virtual machine state, configuration
- `tests`: initial core assertions
- `config`: application, database, machine, and plugin YAML
- `database/schema.sql`: initial MySQL schema
- `ui`: CSS layout and theme files
- `docs/Architecture`: architecture note
- `schematics`: system diagram sources
- `plugins`: plugin design notes

## Verified Repository Findings

- CMake declares a C++23 `lcs_core` library, CLI target, desktop target, and CTest target.
- The core currently implements basic vector math (magnitude, clamp, linear interpolation), a string-keyed in-memory configuration map, and a small virtual-machine lifecycle.
- The initial virtual-machine snapshot carries state, position, temperature, and pressure.
- The MySQL schema defines machines, sensors, sensor readings, and an audit log.
- Physical machine control is disabled in configuration and simulation is the stated default.
- The test suite contains a small set of assertions but has not been verified by this audit in a Windows build.
- The repository's `LithographyControlStudio-Cpp23-Foundation.zip` is only 166 bytes and is documented as a placeholder; do not treat it as a usable source archive.

## Priority Roadmap

### P0 — Reproducible build and CI
1. Add Windows CI using Visual Studio/MSVC and CMake.
2. Run configure, compile, and CTest on each pull request.
3. Add formatting and static analysis checks.
4. Publish truthful build status and attach build artifacts only when generated successfully.

### P1 — Core correctness
1. Specify legal state transitions (Offline, Ready, Running, Paused, Fault).
2. Add error/result types and explicit fault handling.
3. Validate finite numeric inputs and reject invalid values.
4. Test state transitions, boundary cases, and error paths.
5. Ensure simulation state cannot be confused with real hardware state.

### P1 — Configuration and secrets
1. Validate YAML schemas at startup.
2. Keep credentials out of tracked configuration; read secrets from environment or a secret manager.
3. Provide a safe example environment file containing placeholders only.
4. Fail closed if required configuration is missing or invalid.

### P2 — Data persistence
1. Add a migration runner and versioned SQL migrations.
2. Integrate a maintained MySQL client through an isolated persistence adapter.
3. Use prepared statements, bounded connection pools, transaction handling, and least-privilege DB accounts.
4. Add retention and indexing strategy for telemetry.
5. Record security-relevant actions and configuration changes in an append-only audit trail where practical.

### P2 — Application services and interface
1. Define service APIs for machine snapshots, configuration, telemetry, alerts, and simulation controls.
2. Keep UI presentation separate from core logic.
3. Implement charts and status panels using mock/simulated data first.
4. Add accessible keyboard navigation and clear alarm states.

### P3 — Plugin architecture
1. Define a stable plugin interface and version compatibility contract.
2. Require explicit allowlisting and signed artifacts where plugin trust is required.
3. Avoid loading untrusted dynamic libraries in the main process.
4. Keep hardware drivers isolated and disabled by default.

### P3 — Simulation validation
1. Document the simulation's assumptions and units.
2. Separate illustrative values from calibrated physical models.
3. Add deterministic scenarios and replayable fixtures.
4. Do not claim process accuracy until validated against approved reference data.

## Safety Boundary

This roadmap is for a simulation-first engineering application. It does not provide bypasses for interlocks or instructions for controlling production lithography equipment. Hardware integration should remain unavailable until authorized interfaces, hazard analysis, access control, logging, and vendor-approved safeguards are in place.

## Validation Status

Repository files were inspected through GitHub on 2026-10-23. No local compilation, Windows execution, MySQL integration test, hardware test, or release packaging was performed during this audit.

# Lithography Control Studio

Simulation-first C++23 engineering foundation for machine-state modeling, telemetry, alarms, configuration, and future authorized integrations.

## Safety boundary

This repository does not implement production lithography equipment control, interlock bypasses, or vendor-protocol commands. The default backend is a virtual simulator and physical control is disabled.

## Build

Windows: `cmake --preset windows-msvc-release && cmake --build build/release --config Release --parallel && ctest --test-dir build/release -C Release --output-on-failure`

Linux/macOS: `cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build --parallel && ctest --test-dir build --output-on-failure`

## Components

- `lcs_core`: domain, simulation, telemetry, alarms, configuration
- `lcs_cli`: command-line simulation shell
- `lcs_desktop`: desktop shell placeholder
- `database/`: MySQL schema and migrations
- `config/`: application and simulation configuration
- `docs/`: architecture and operations documentation
- `.github/workflows/`: Windows CI

## Database

Copy `.env.example` to `.env`, replace placeholders, then start MySQL with Docker Compose. Never commit `.env`.

## Status

This is an engineering foundation. Native GUI, production-grade database adapter, plugin ABI, and validated physical integrations remain future work.

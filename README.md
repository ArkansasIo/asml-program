# Lithography Control Studio

Simulation-first C++23 engineering foundation for machine-state modeling, telemetry, alarms, configuration, and future authorized integrations.

## Safety boundary

This repository does not implement production lithography equipment control, interlock bypasses, or vendor-protocol commands. The default backend is a virtual simulator and physical control is disabled.

## Build

Windows: `cmake --preset windows-msvc-release && cmake --build build/release --config Release --parallel && ctest --test-dir build/release -C Release --output-on-failure`

Linux/macOS: `cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build --parallel && ctest --test-dir build --output-on-failure`

## Components

- `lcs_core`: domain, simulation, telemetry, alarms, configuration, control planes, REST API HTTP server
- `lcs_cli`: command-line simulation shell
- `lcs_desktop`: desktop shell placeholder
- `lcs_api`: REST API server serving the web UI and control-plane endpoints
- `ui/`: browser control UI (`index.html`, `app.js`) styled by `ui/layout/` and `ui/themes/`
- `database/`: MySQL schema and migrations
- `config/`: application and simulation configuration
- `docs/`: architecture and operations documentation
- `.github/workflows/`: Windows CI
## Control planes
`include/lcs/ControlPlanes.hpp` defines the layered control surface over the simulation backend:

- **Safety** — interlock registry; every plane gates its commands on it
- **Machine** — init/start/pause/stop lifecycle over the virtual machine
- **Stage** — absolute/relative moves with soft-limit validation
- **Optics** — parameter validation with resolution readback
- **Alarms** — raise/clear with critical latching
- **Telemetry** — bounded sample capture with drain
- **Jobs** — exposure job lifecycle (create → validate → queue → run → complete)
- **Config** — validated key/value settings
Every accepted command is recorded in a bounded audit trail.

## REST API & UI
Run from the project root:

```
build/release/Release/lcs_api.exe 8080
```

Then open `http://127.0.0.1:8080` for the browser UI. Key endpoints:

| Method | Path | Description |
|---|---|---|
| GET | `/api/state` | machine snapshot |
| POST | `/api/init` `/api/start` `/api/pause` `/api/stop` | lifecycle commands |
| POST | `/api/move` | stage move, JSON body `{"x":..,"y":..,"z":..}` |
| GET/POST | `/api/safety`, `/api/safety/set` | interlock status / updates |
| GET/POST | `/api/optics`, `/api/optics/set` | optics readback / parameters |
| GET/POST | `/api/alarms`, `/api/alarms/raise`, `/api/alarms/clear` | alarm plane |
| POST/GET | `/api/telemetry/push`, `/api/telemetry/drain` | telemetry plane |
| POST | `/api/jobs/create`, `/api/jobs/<id>/<action>` | job plane |
| POST/GET | `/api/config/set`, `/api/config/get` | config plane |
| GET | `/api/audit` | command audit trail |
| GET | `/api/math/mag?x=&y=&z=` | vector magnitude |

The server binds to loopback only and exposes simulation state — no physical control.

## Database

Copy `.env.example` to `.env`, replace placeholders, then start MySQL with Docker Compose. Never commit `.env`.

## Status

This is an engineering foundation. Native GUI, production-grade database adapter, plugin ABI, and validated physical integrations remain future work.

Control planes, REST API, and browser UI are implemented against the simulation backend only.

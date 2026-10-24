# Simulation Domain

## Purpose

The simulation domain provides deterministic, vendor-neutral models for software development and testing. It must not be interpreted as a production lithography controller.

## Modules

- `Units`: explicit unit conversions for length, pressure, and temperature.
- `StageModel`: bounded XYZ position model with finite-value validation.
- `WaferModel`: deterministic rectangular site generation inside a configured wafer envelope.
- `OpticsModel`: illustrative resolution calculation using documented educational assumptions.
- `JobModel`: in-memory job/exposure lifecycle and validation.
- `SafetyInterlock`: fail-closed software gate used by simulations and tests.

## Design rules

1. Simulation state is distinct from physical equipment state.
2. Inputs are validated before state mutation.
3. Units are explicit at API boundaries.
4. No vendor protocol, machine command, or interlock bypass is implemented.
5. Models remain deterministic so CTest can exercise them without external services.

## Extension points

Future work can add persistence adapters, a scheduler, UI view models, deterministic replay, and plugin interfaces. Any hardware-facing implementation must remain isolated behind an explicitly disabled integration boundary and use authorized vendor documentation and safeguards.

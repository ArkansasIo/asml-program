# Operations Guide

## Simulation

Simulation is the only default execution mode. Start `lcs_cli` or `lcs_desktop` after building.

## Telemetry

Telemetry buffers are bounded to prevent unbounded memory growth. Persisted telemetry should use the database adapter and retention policy.

## Alarms

Critical alarms should block future hardware-control activation until cleared by an authorized service.

## Database

Use numbered forward-only migrations. Application accounts should use least privilege. Do not commit passwords or production connection strings.

## Production readiness

Before real equipment integration, complete vendor interface review, hazard analysis, access control, signed plugin verification, audit logging, failure-mode testing, and independent safety validation.

# Logging Improvements (Sim Debugging)

## Goals

- Make sim runtime behavior observable with consistent, structured logs.
- Provide actionable debug output without overwhelming default runs.
- Support post-mortem debugging (trace buffers, failure dumps, and log capture).

## Scope

- Logging backend selection and a small project-wide logging facade.
- Sim-specific logging policy (what to log, when, and at which level).
- CLI/log configuration for `irata2_run` and tests.
- Structured fields for CPU state, controls, buses, and microcode steps.

## Non-Goals

- Replacing the existing debug dump/trace buffer (they remain).
- Full tracing for every control line on every tick by default (too noisy).

## Proposed Design

### 1) Logging Backend + Facade

- Use `absl::log` as the backend.
- Provide a minimal wrapper (`irata2::log`) so call sites stay stable if
  the backend changes later.
- Standardize log levels: `INFO`, `WARNING`, `ERROR`, `DEBUG`.

### 2) Log Events (Sim)

**Core events**
- `sim.start`: cartridge path, entry PC, trace depth, debug sidecar path.
- `sim.halt`: halt reason, cycle count, instruction address.
- `sim.crash`: crash reason, cycle count, instruction address.
- `sim.timeout`: max cycles exceeded with last instruction address.

**Optional debug events**
- `sim.tick`: cycle count, phase, PC, IPC, IR, SC.
- `sim.trace`: emitted on failure; includes trace buffer content.
- `sim.dump`: emitted on failure; includes formatted debug dump.

### 3) Structured Fields

Introduce a small field helper that consistently prints:
- `pc`, `ipc`, `ir`, `sc`
- `a`, `x`, `sr` and decoded flags
- `bus.data`, `bus.address`
- `instruction.address`, `instruction.source`

### 4) Configuration + CLI

- Add `--log-level {info,debug,warning,error}` to `irata2_run`.
- Default to `info` for summaries; `debug` for detailed per-tick logging.
- Provide an environment override (`IRATA2_LOG_LEVEL`) for tests/CI.

### 5) Integration with Existing Debugging

- On failure, log `sim.dump` and `sim.trace` at `info`.
- Keep `FormatDebugDump()` as the canonical formatting for dumps.
- Reuse debug symbols to annotate source locations in logs.

## Detailed Plan

1) **Facade + Backend**
   - Add a lightweight logging wrapper in `base` or `sim`.
   - Wire to `absl::log` with an internal adapter.
   - Add tests for log level gating (if feasible).

2) **Sim Event Coverage**
   - Add `sim.start`, `sim.halt`, `sim.crash`, `sim.timeout` events.
   - Add optional `sim.tick` gated by debug level.

3) **Failure Path Logging**
   - Log formatted dump and trace on failure using `info` level.
   - Ensure `--debug` and `--trace-depth` drive richer output.

4) **CLI + Config**
   - Add `--log-level` to `irata2_run`.
   - Add `IRATA2_LOG_LEVEL` env override.

5) **Tests + Docs**
   - Add unit tests for `irata2_run` log-level handling.
   - Update `sim/README.md` with logging usage and examples.

## Milestones

- M0: Logging facade + `absl::log` integration.
- M1: Sim lifecycle logs (`start`, `halt`, `crash`, `timeout`).
- M2: Failure-path logs (`dump`, `trace`) at info level.
- M3: CLI log-level configuration and env override.
- M4: Documentation + test coverage for logging.

## Open Questions

- Should `sim.tick` log once per instruction (IPC latch) instead of per cycle?
- Do we want JSON-formatted logs for machine parsing, or keep plain text?

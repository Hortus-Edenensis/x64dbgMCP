# x64dbgMCP Project Safety Model (Phase A)

Use this note for repository-specific behavior that extends generic x64dbg command knowledge.

## Elevation Modes

- Environment variable: `X64DBG_ELEVATION_MODE=off|warn|enforce`
- Current default: `warn`
- Server behavior:
  - `off`: dangerous actions run without elevation checks (unless safe mode is also enabled).
  - `warn`: dangerous actions without `confirm=1` are allowed but warnings are emitted.
  - `enforce`: dangerous actions without `confirm=1` are rejected.

## Command Risk Classification

- `/ExecCommand` uses risk classification:
  - `whitelist` read-only commands: pass through directly.
  - `dangerous` commands: elevation confirmation flow applies.
- Classification logic is implemented in:
  - C++: `src/ServerLogic.cpp` (`ClassifyExecCommandRisk`)
  - Python: `src/x64dbg.py` (`classify_exec_command_risk`)
- Keep both lists synchronized when updating policy.

## Write Endpoints Under Elevation Gate

The following endpoints are explicitly gated by dangerous-action confirmation:

- `/Register/Set`
- `/Memory/Write`
- `/Debug/Stop`
- `/Debug/SetBreakpoint`
- `/Debug/DeleteBreakpoint`
- `/Cmdline/Set`
- `/Assembler/AssembleMem`
- `/Stack/Pop`
- `/Stack/Push`
- `/Flag/Set`

## Python Wrapper Semantics

- `ExecCommand` enforces client-side confirm for dangerous commands:
  - If dangerous and `confirm=False`, it returns a readable elevation error before HTTP call.
- `CommandRun` follows `ExecCommand` behavior and surfaces elevation failures through `error`.
- `SetRegister` now routes to `RegisterSet` (no raw `r reg=value` command construction).
- `main_cli` resolves tools from MCP registry only (no arbitrary `globals()` execution).

## Operational Guidance

- For deterministic automation, treat `warn` mode as if it were `enforce`: always pass confirm on dangerous/write operations.
- For write workflows:
  1. pause debugger,
  2. apply write with confirm,
  3. verify result and capture state for rollback if needed.

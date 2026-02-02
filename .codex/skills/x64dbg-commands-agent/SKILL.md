---
name: x64dbg-commands-agent
description: Use when implementing or mapping x64dbg command skills/agents, wrapping command execution safely, or translating documented x64dbg commands into MCP tool usage and safe workflows.
---

# x64dbg Commands Agent

## Scope
Use this skill to turn x64dbg command documentation into concrete MCP workflows. Prioritize safety wrappers (pause/confirm/rollback) and prefer explicit VA expressions.

## References
- Read `references/commands.md` for command lists, syntax, and wrapper guidance.

## Workflow
1. Identify the command category and exact command name from the reference.
2. Decide if the operation is write/destructive; if yes, require pause + confirmation.
3. Prefer `CommandRun` (Python wrapper) for safe execution with optional snapshots.
4. For reads, prefer detailed APIs (e.g., `MemoryReadDetailed`) when partial reads matter.
5. For pause/step: call `DebugPause(wait=true)` and use `DebugStep*` with `auto_pause=true` to avoid “Debugger running” errors.
6. For run‑to‑user: call `Debug/RunUntilUserCode` or `ExecCommand("RunToUserCode")`; re‑check RIP via `DisasmGetInstructionAtRIP`.

## Safety defaults
- Pause before writes (register/memory/flags/breakpoints).
- Use `confirm=true` for write endpoints in safe mode (register/memory/flags/cmdline/stack/asm/stop).
- Use `dry_run` or `require_confirm` when risky.
- Capture before/after state if values may need rollback.

## Notes
- Keep SKILL.md lean; load `references/commands.md` only when needed.

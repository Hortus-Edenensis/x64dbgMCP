---
name: x64dbg-commands-agent
description: Use when implementing or mapping x64dbg command skills/agents, wrapping command execution safely, translating documented x64dbg commands into MCP tool usage, or enforcing x64dbgMCP Phase A elevation policy (whitelist direct pass + dangerous confirm gating).
---

# x64dbg Commands Agent

## Scope
Use this skill to turn x64dbg command documentation into concrete MCP workflows for `x64dbgMCP`. Prioritize deterministic safety wrappers (pause/confirm/rollback) and explicit VA expressions.

## References
- Read `references/commands.md` for command lists, syntax, and wrapper guidance.
- Read `references/project-safety-model.md` when working in this repository to follow the current elevation and endpoint behavior.

## Workflow
1. Identify the command category and exact command name from the reference.
2. Classify risk before execution:
   - `whitelist` read-only command: allow direct execution.
   - `dangerous` command: require explicit confirmation.
3. Prefer typed MCP tools over raw command strings for writes (`RegisterSet`, `MemoryWrite`, `FlagSet`, `CmdlineSet`, `AssemblerAssembleMem`, `DebugSetBreakpoint`, `DebugDeleteBreakpoint`).
4. For `ExecCommand` / `CommandRun`, set `confirm=true` for dangerous commands; treat missing-confirm errors as elevation policy failure, not transport failure.
5. Pause before writes: call `DebugPause(wait=true, timeout_ms=30000)` and then execute write operations.
6. For reads, prefer detailed APIs (for example `MemoryReadDetailed`) when partial reads or metadata matter.
7. For run-to-user flow, prefer `Debug/RunUntilUserCode(wait=true, timeoutMs=30000, pauseFirst=true)` and then re-check RIP with `DisasmGetInstructionAtRIP`.

## Safety defaults
- Pause before writes (register/memory/flags/breakpoints/cmdline/assembler).
- Use `confirm=true` for dangerous commands and all write endpoints by default.
- Use `dry_run` or `require_confirm` for high-risk workflows and batch operations.
- Capture before/after state if values may need rollback.

## Notes
- Keep `src/ServerLogic.cpp` and `src/x64dbg.py` risk classification tables in sync when command policy changes.
- Keep SKILL.md lean; load reference files only when needed.

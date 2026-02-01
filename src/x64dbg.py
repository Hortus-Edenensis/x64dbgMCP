import sys
import os
import inspect
import json
import time
from typing import Any, Dict, List, Callable
import requests

from mcp.server.fastmcp import FastMCP

DEFAULT_X64DBG_SERVER = "http://127.0.0.1:8888/"
DEFAULT_REQUEST_TIMEOUT_SEC = 30

def _read_int_env(name: str, default: int) -> int:
    raw = os.getenv(name)
    if not raw:
        return default
    try:
        return int(raw)
    except Exception:
        return default

def _read_float_env(name: str, default: float) -> float:
    raw = os.getenv(name)
    if not raw:
        return default
    try:
        return float(raw)
    except Exception:
        return default

DEFAULT_EXEC_WAIT_SEC = _read_int_env("X64DBG_EXEC_WAIT_SEC", 30)
DEFAULT_EXEC_POLL_SEC = _read_float_env("X64DBG_EXEC_POLL_SEC", 0.2)

def _resolve_server_url_from_args_env() -> str:
    env_url = os.getenv("X64DBG_URL")
    if env_url and env_url.startswith("http"):
        return env_url
    if len(sys.argv) > 1 and isinstance(sys.argv[1], str) and sys.argv[1].startswith("http"):
        return sys.argv[1]
    return DEFAULT_X64DBG_SERVER

x64dbg_server_url = _resolve_server_url_from_args_env()

def set_x64dbg_server_url(url: str) -> None:
    global x64dbg_server_url
    if url and url.startswith("http"):
        x64dbg_server_url = url

mcp = FastMCP("x64dbg-mcp")

def safe_get(endpoint: str, params: dict = None):
    """
    Perform a GET request with optional query parameters.
    Returns parsed JSON if possible, otherwise text content
    """
    if params is None:
        params = {}

    url = f"{x64dbg_server_url}{endpoint}"

    try:
        response = requests.get(url, params=params, timeout=DEFAULT_REQUEST_TIMEOUT_SEC)
        response.encoding = 'utf-8'
        if response.ok:
            content_type = response.headers.get("Content-Type", "").lower()
            if "application/json" in content_type:
                try:
                    return response.json()
                except ValueError:
                    return response.text.strip()
            return response.text.strip()
        return f"Error {response.status_code}: {response.text.strip()}"
    except Exception as e:
        return f"Request failed: {str(e)}"

def safe_post(endpoint: str, data: dict | str):
    """
    Perform a POST request with data.
    Returns parsed JSON if possible, otherwise text content
    """
    try:
        url = f"{x64dbg_server_url}{endpoint}"
        if isinstance(data, dict):
            response = requests.post(url, data=data, timeout=DEFAULT_REQUEST_TIMEOUT_SEC)
        else:
            response = requests.post(url, data=data.encode("utf-8"), timeout=DEFAULT_REQUEST_TIMEOUT_SEC)
        
        response.encoding = 'utf-8'
        if response.ok:
            content_type = response.headers.get("Content-Type", "").lower()
            if "application/json" in content_type:
                try:
                    return response.json()
                except ValueError:
                    return response.text.strip()
            return response.text.strip()
        return f"Error {response.status_code}: {response.text.strip()}"
    except Exception as e:
        return f"Request failed: {str(e)}"

def _is_error_response(result: Any) -> bool:
    if isinstance(result, dict) and "error" in result:
        return True
    if isinstance(result, str):
        lower = result.lower()
        return lower.startswith("error") or lower.startswith("request failed")
    return False

def _split_csv(text: str) -> List[str]:
    if not text:
        return []
    return [item.strip() for item in text.split(",") if item.strip()]

def _parse_memory_spec(text: str) -> List[Dict[str, str]]:
    items = _split_csv(text)
    result = []
    for item in items:
        if ":" not in item:
            continue
        addr, size = item.split(":", 1)
        addr = addr.strip()
        size = size.strip()
        if addr and size:
            result.append({"addr": addr, "size": size})
    return result

def _get_is_running() -> tuple[bool | None, str | None]:
    res = safe_get("IsDebugActive")
    if isinstance(res, dict) and "isRunning" in res:
        return bool(res["isRunning"]), None
    if isinstance(res, dict) and "error" in res:
        return None, str(res["error"])
    if isinstance(res, str) and _is_error_response(res):
        return None, res
    return None, "Unexpected IsDebugActive response"

def _get_is_debugging() -> tuple[bool | None, str | None]:
    res = safe_get("Is_Debugging")
    if isinstance(res, dict) and "isDebugging" in res:
        return bool(res["isDebugging"]), None
    if isinstance(res, dict) and "error" in res:
        return None, str(res["error"])
    if isinstance(res, str) and _is_error_response(res):
        return None, res
    return None, "Unexpected IsDebugging response"

def _capture_state(registers: List[str], memory_specs: List[Dict[str, str]], capture_ip: bool) -> Dict[str, Any]:
    state: Dict[str, Any] = {"registers": {}, "memory": []}

    regs = list(registers)
    if capture_ip:
        if "rip" not in regs and "eip" not in regs:
            regs.insert(0, "rip")

    for reg in regs:
        value = safe_get("Register/Get", {"register": reg})
        state["registers"][reg] = value
        if reg == "rip" and isinstance(value, str) and value.lower().startswith("unknown"):
            value = safe_get("Register/Get", {"register": "eip"})
            state["registers"]["eip"] = value

    for spec in memory_specs:
        addr = spec.get("addr", "")
        size = spec.get("size", "")
        if not addr or not size:
            continue
        data = safe_get("Memory/ReadDetailed", {"addr": addr, "size": size})
        state["memory"].append({"addr": addr, "size": size, "data": data})

    return state

def _normalize_windows_path(path: str) -> str:
    if not path:
        return ""
    return path.replace("/", "\\").lower()

def _is_system_module(module_name: str | None, module_path: str | None, system_root: str) -> bool:
    name = (module_name or "").lower()
    path = _normalize_windows_path(module_path or "")
    root = _normalize_windows_path(system_root)

    if not name and not path:
        return True

    if path and root and path.startswith(root):
        return True
    if "\\windows\\" in path:
        return True

    if name:
        system_names = {
            "ntdll.dll",
            "kernel32.dll",
            "kernelbase.dll",
            "user32.dll",
            "gdi32.dll",
            "gdi32full.dll",
            "win32u.dll",
            "advapi32.dll",
            "ucrtbase.dll",
            "msvcrt.dll",
            "msvcp140.dll",
            "vcruntime140.dll",
            "vcruntime140_1.dll",
            "combase.dll",
            "ole32.dll",
            "oleaut32.dll",
            "rpcrt4.dll",
            "sechost.dll",
            "shlwapi.dll",
            "shcore.dll",
            "imm32.dll",
            "ws2_32.dll",
            "inputhost.dll",
        }
        if name in system_names:
            return True

    return False

# =============================================================================
# TOOL REGISTRY INTROSPECTION (for CLI/Claude tool-use)
# =============================================================================

def _get_mcp_tools_registry() -> Dict[str, Callable[..., Any]]:
    """
    Build a registry of available MCP-exposed tool callables in this module.
    Heuristic: exported callables starting with an uppercase letter.
    """
    registry: Dict[str, Callable[..., Any]] = {}
    for name, obj in globals().items():
        if not name or not name[0].isupper():
            continue
        if callable(obj):
            try:
                # Validate signature to ensure it's a plain function
                inspect.signature(obj)
                registry[name] = obj
            except (TypeError, ValueError):
                pass
    return registry

def _describe_tool(name: str, func: Callable[..., Any]) -> Dict[str, Any]:
    sig = inspect.signature(func)
    params = []
    for p in sig.parameters.values():
        if p.kind in (inspect.Parameter.POSITIONAL_ONLY, inspect.Parameter.VAR_POSITIONAL, inspect.Parameter.VAR_KEYWORD):
            # Skip non-JSON friendly params in schema
            continue
        params.append({
            "name": p.name,
            "required": p.default is inspect._empty,
            "type": "string" if p.annotation in (str, inspect._empty) else ("boolean" if p.annotation is bool else ("integer" if p.annotation is int else "string"))
        })
    return {
        "name": name,
        "description": (func.__doc__ or "").strip(),
        "params": params
    }

def _list_tools_description() -> List[Dict[str, Any]]:
    reg = _get_mcp_tools_registry()
    return [_describe_tool(n, f) for n, f in sorted(reg.items(), key=lambda x: x[0].lower())]

def _invoke_tool_by_name(name: str, args: Dict[str, Any]) -> Any:
    reg = _get_mcp_tools_registry()
    if name not in reg:
        return {"error": f"Unknown tool: {name}"}
    func = reg[name]
    try:
        # Prefer keyword invocation; convert all values to strings unless bool/int expected
        sig = inspect.signature(func)
        bound_kwargs: Dict[str, Any] = {}
        for p in sig.parameters.values():
            if p.kind in (inspect.Parameter.VAR_POSITIONAL, inspect.Parameter.VAR_KEYWORD, inspect.Parameter.POSITIONAL_ONLY):
                continue
            if p.name in args:
                value = args[p.name]
                # Simple coercions for common types
                if p.annotation is bool and isinstance(value, str):
                    value = value.lower() in ("1", "true", "yes", "on")
                elif p.annotation is int and isinstance(value, str):
                    try:
                        value = int(value, 0)
                    except Exception:
                        try:
                            value = int(value)
                        except Exception:
                            pass
                bound_kwargs[p.name] = value
        return func(**bound_kwargs)
    except Exception as e:
        return {"error": str(e)}

# =============================================================================
# Claude block normalization helpers
# =============================================================================

def _block_to_dict(block: Any) -> Dict[str, Any]:
    try:
        # Newer anthropic SDK objects are Pydantic models
        if hasattr(block, "model_dump") and callable(getattr(block, "model_dump")):
            return block.model_dump()
    except Exception:
        pass
    if isinstance(block, dict):
        return block
    btype = getattr(block, "type", None)
    if btype == "text":
        return {"type": "text", "text": getattr(block, "text", "")}
    if btype == "tool_use":
        return {
            "type": "tool_use",
            "id": getattr(block, "id", None),
            "name": getattr(block, "name", None),
            "input": getattr(block, "input", {}) or {},
        }
    # Fallback generic representation
    return {"type": str(btype or "unknown"), "raw": str(block)}

# =============================================================================
# UNIFIED COMMAND EXECUTION
# =============================================================================

@mcp.tool()
def ExecCommand(cmd: str, wait: bool = False, confirm: bool = False) -> str:
    """
    Execute a command in x64dbg and return its output
    
    Parameters:
        cmd: Command to execute
        wait: Wait for command completion and capture output
    
    Returns:
        Command execution status and output
    """
    params = {"cmd": cmd, "wait": "1" if wait else "0"}
    if confirm:
        params["confirm"] = "1"
    result = safe_get("ExecCommand", params)
    if not wait:
        return result
    if _is_error_response(result):
        return result
    if isinstance(result, dict) and "job_id" in result:
        job_id = result["job_id"]
        deadline = time.time() + max(DEFAULT_EXEC_WAIT_SEC, 1)
        poll_delay = max(DEFAULT_EXEC_POLL_SEC, 0.05)
        while time.time() < deadline:
            job = safe_get("ExecCommand/Result", {"id": job_id})
            if _is_error_response(job):
                return job
            if isinstance(job, dict):
                status = job.get("status")
                if status in ("done", "failed"):
                    return job.get("output") or job.get("error") or ""
            time.sleep(poll_delay)
        return {"error": "Timed out waiting for command result", "job_id": job_id}
    return result

@mcp.tool()
def ExecCommandResult(job_id: str, consume: bool = False) -> Any:
    """
    Retrieve the result of an async ExecCommand job

    Parameters:
        job_id: Job identifier returned by ExecCommand(wait=True)
        consume: Remove the job entry after completion

    Returns:
        Job status and output
    """
    params = {"id": job_id}
    if consume:
        params["consume"] = "1"
    return safe_get("ExecCommand/Result", params)

@mcp.tool()
def CommandRun(cmd: str,
               wait: bool = False,
               dry_run: bool = False,
               require_confirm: bool = False,
               confirm: bool = False,
               ensure_paused: bool = False,
               auto_pause: bool = False,
               capture_before: bool = False,
               capture_after: bool = False,
               capture_ip: bool = True,
               registers: str = "",
               memory: str = "") -> Dict[str, Any]:
    """
    Safe command execution wrapper with optional dry-run, confirmation, and snapshots.

    Parameters:
        cmd: Command to execute
        wait: Wait for command completion
        dry_run: Only return plan, do not execute
        require_confirm: Require confirm=True to execute
        confirm: Confirmation flag
        ensure_paused: Ensure debugger is paused before execution
        auto_pause: Attempt to pause if running
        capture_before: Capture state before execution
        capture_after: Capture state after execution
        capture_ip: Include instruction pointer in snapshot
        registers: Comma-separated register names to capture
        memory: Comma-separated memory specs: "addr:size"

    Returns:
        Structured result with output and snapshots
    """
    result: Dict[str, Any] = {"cmd": cmd, "executed": False}

    if not cmd:
        result["error"] = "Missing command"
        return result

    is_debugging, dbg_err = _get_is_debugging()
    if is_debugging is False:
        result["error"] = "No active debug session"
        return result
    if dbg_err:
        result["error"] = dbg_err
        return result

    if ensure_paused:
        is_running, run_err = _get_is_running()
        if run_err:
            result["error"] = run_err
            return result
        if is_running:
            if auto_pause:
                safe_get("Debug/Pause")
                deadline = time.time() + 3.0
                while time.time() < deadline:
                    is_running, run_err = _get_is_running()
                    if run_err:
                        break
                    if not is_running:
                        break
                    time.sleep(0.05)
            else:
                result["error"] = "Debugger is running; pause required"
                return result

            is_running, run_err = _get_is_running()
            if run_err:
                result["error"] = run_err
                return result
            if is_running:
                result["error"] = "Failed to pause debugger"
                return result

    regs_list = _split_csv(registers)
    mem_list = _parse_memory_spec(memory)

    if capture_before:
        result["before"] = _capture_state(regs_list, mem_list, capture_ip)

    if dry_run:
        result["dry_run"] = True
        result["rollback_plan"] = result.get("before")
        return result

    if require_confirm and not confirm:
        result["error"] = "Confirmation required"
        result["rollback_plan"] = result.get("before")
        return result

    output = ExecCommand(cmd=cmd, wait=wait, confirm=confirm)
    result["output"] = output
    if _is_error_response(output):
        result["error"] = output
        return result

    result["executed"] = True

    if capture_after:
        result["after"] = _capture_state(regs_list, mem_list, capture_ip)
    if capture_before:
        result["rollback_plan"] = result.get("before")

    return result

# =============================================================================
# DEBUGGING STATUS
# =============================================================================

@mcp.tool()
def IsDebugActive() -> bool:
    """
    Check if debugger is active (running)

    Returns:
        True if running, False otherwise
    """
    result = safe_get("IsDebugActive")
    if isinstance(result, dict) and "error" in result:
        return result
    if isinstance(result, str) and _is_error_response(result):
        return {"error": result}
    if isinstance(result, dict) and "isRunning" in result:
        return result["isRunning"] is True
    if isinstance(result, str):
        try:
            import json
            parsed = json.loads(result)
            return parsed.get("isRunning", False) is True
        except Exception:
            return False
    return False

@mcp.tool()
def IsDebugging() -> bool:
    """
    Check if x64dbg is debugging a process

    Returns:
        True if debugging, False otherwise
    """
    result = safe_get("Is_Debugging")
    if isinstance(result, dict) and "error" in result:
        return result
    if isinstance(result, str) and _is_error_response(result):
        return {"error": result}
    if isinstance(result, dict) and "isDebugging" in result:
        return result["isDebugging"] is True
    if isinstance(result, str):
        try:
            import json
            parsed = json.loads(result)
            return parsed.get("isDebugging", False) is True
        except Exception:
            return False
    return False
# =============================================================================
# REGISTER API
# =============================================================================

@mcp.tool()
def RegisterGet(register: str) -> str:
    """
    Get register value using Script API
    
    Parameters:
        register: Register name (e.g. "eax", "rax", "rip")
    
    Returns:
        Register value in hex format
    """
    return safe_get("Register/Get", {"register": register})

@mcp.tool()
def RegisterSet(register: str, value: str, confirm: bool = False) -> str:
    """
    Set register value using Script API
    
    Parameters:
        register: Register name (e.g. "eax", "rax", "rip")
        value: Value to set (in hex format, e.g. "0x1000")
        confirm: Confirm write when safe mode is enabled
    
    Returns:
        Status message
    """
    params = {"register": register, "value": value}
    if confirm:
        params["confirm"] = "1"
    return safe_get("Register/Set", params)

# =============================================================================
# MEMORY API (Enhanced)
# =============================================================================

@mcp.tool()
def MemoryRead(addr: str, size: str) -> str:
    """
    Read memory using enhanced Script API
    
    Parameters:
        addr: Memory address (in hex format, e.g. "0x1000")
        size: Number of bytes to read
    
    Returns:
        Hexadecimal string representing the memory contents
    """
    detailed = safe_get("Memory/ReadDetailed", {"addr": addr, "size": size})
    if isinstance(detailed, dict):
        data = detailed.get("data")
        if data is not None:
            return data
        return json.dumps(detailed, ensure_ascii=True)
    if isinstance(detailed, str):
        return detailed
    if isinstance(detailed, int):
        return f"{detailed:x}"
    return str(detailed)

@mcp.tool()
def MemoryReadDetailed(addr: str, size: str) -> Any:
    """
    Read memory and return detailed metadata (partial reads, bytes count)

    Parameters:
        addr: Memory address (in hex format, e.g. "0x1000")
        size: Number of bytes to read

    Returns:
        JSON with data/bytes/partial or error
    """
    return safe_get("Memory/ReadDetailed", {"addr": addr, "size": size})

@mcp.tool()
def MemoryWrite(addr: str, data: str, confirm: bool = False) -> str:
    """
    Write memory using enhanced Script API
    
    Parameters:
        addr: Memory address (in hex format, e.g. "0x1000")
        data: Hexadecimal string representing the data to write
        confirm: Confirm write when safe mode is enabled
    
    Returns:
        Status message
    """
    params = {"addr": addr, "data": data}
    if confirm:
        params["confirm"] = "1"
    return safe_get("Memory/Write", params)

@mcp.tool()
def MemoryIsValidPtr(addr: str) -> bool:
    """
    Check if memory address is valid
    
    Parameters:
        addr: Memory address (in hex format, e.g. "0x1000")
    
    Returns:
        True if valid, False otherwise
    """
    result = safe_get("Memory/IsValidPtr", {"addr": addr})
    if isinstance(result, str):
        return result.lower() == "true"
    return False

@mcp.tool()
def MemoryGetProtect(addr: str) -> str:
    """
    Get memory protection flags
    
    Parameters:
        addr: Memory address (in hex format, e.g. "0x1000")
    
    Returns:
        Protection flags in hex format
    """
    return safe_get("Memory/GetProtect", {"addr": addr})

# =============================================================================
# DEBUG API
# =============================================================================

@mcp.tool()
def DebugRun() -> str:
    """
    Resume execution of the debugged process using Script API
    
    Returns:
        Status message
    """
    return safe_get("Debug/Run")

@mcp.tool()
def DebugRestart() -> str:
    """
    Restart the debugged process using the debugger command queue

    Returns:
        Status message
    """
    return safe_get("Debug/Restart")

@mcp.tool()
def DebugPause() -> str:
    """
    Pause execution of the debugged process using Script API
    
    Returns:
        Status message
    """
    return safe_get("Debug/Pause")

@mcp.tool()
def DebugStop(confirm: bool = False) -> str:
    """
    Stop debugging using Script API

    Parameters:
        confirm: Confirm stop when safe mode is enabled
    
    Returns:
        Status message
    """
    params = {}
    if confirm:
        params["confirm"] = "1"
    return safe_get("Debug/Stop", params)

@mcp.tool()
def DebugStepIn() -> str:
    """
    Step into the next instruction using Script API
    
    Returns:
        Status message
    """
    return safe_get("Debug/StepIn")

@mcp.tool()
def DebugStepOver() -> str:
    """
    Step over the next instruction using Script API
    
    Returns:
        Status message
    """
    return safe_get("Debug/StepOver")

@mcp.tool()
def DebugStepOverN(count: int) -> str:
    """
    Step over multiple instructions using the debugger command queue

    Parameters:
        count: Number of step-over iterations

    Returns:
        Status message
    """
    return safe_get("Debug/StepOverN", {"count": str(count)})

@mcp.tool()
def CancelStepBatch() -> str:
    """
    Cancel an in-progress step-over batch

    Returns:
        Status message
    """
    return safe_get("Debug/CancelStepBatch")

@mcp.tool()
def DebugStepOut() -> str:
    """
    Step out of the current function using Script API
    
    Returns:
        Status message
    """
    return safe_get("Debug/StepOut")

@mcp.tool()
def DebugSetBreakpoint(addr: str) -> str:
    """
    Set breakpoint at address using Script API
    
    Parameters:
        addr: Memory address (in hex format, e.g. "0x1000")
    
    Returns:
        Status message
    """
    return safe_get("Debug/SetBreakpoint", {"addr": addr})

@mcp.tool()
def DebugDeleteBreakpoint(addr: str) -> str:
    """
    Delete breakpoint at address using Script API
    
    Parameters:
        addr: Memory address (in hex format, e.g. "0x1000")
    
    Returns:
        Status message
    """
    return safe_get("Debug/DeleteBreakpoint", {"addr": addr})

# =============================================================================
# CMDLINE API
# =============================================================================

@mcp.tool()
def CmdlineGet() -> str:
    """
    Get the current debuggee command line

    Returns:
        Command line string
    """
    return safe_get("Cmdline/Get")

@mcp.tool()
def CmdlineSet(cmdline: str, confirm: bool = False) -> str:
    """
    Set the debuggee command line

    Parameters:
        cmdline: Full command line string
        confirm: Confirm change when safe mode is enabled

    Returns:
        Status message
    """
    if confirm:
        return safe_get("Cmdline/Set", {"cmdline": cmdline, "confirm": "1"})
    return safe_post("Cmdline/Set", cmdline)

# =============================================================================
# WORKFLOW HELPERS
# =============================================================================

@mcp.tool()
def RunUntilUserCode(max_cycles: int = 50, poll_interval_ms: int = 200, max_wait_ms: int = 30000, auto_resume: bool = True) -> dict:
    """
    Continue execution until execution returns to non-system (user) code.

    Parameters:
        max_cycles: Maximum run/pause cycles to attempt
        poll_interval_ms: Poll interval while waiting for a pause
        max_wait_ms: Max time to wait per run before giving up
        auto_resume: When true, auto-continue on system-module pauses

    Returns:
        Dict with RIP/module info or error
    """
    if not IsDebugging():
        return {"error": "Not debugging"}

    native = safe_get("Debug/RunUntilUserCode", {"autoResume": "1" if auto_resume else "0"})
    if isinstance(native, dict) and native.get("entry"):
        return {"status": "queued", **native}
    if isinstance(native, str) and "Unknown endpoint" not in native and "404" not in native:
        return {"error": native}

    system_root = os.getenv("SystemRoot") or os.getenv("WINDIR") or "C:\\Windows"
    last_info: Dict[str, Any] = {}

    for cycle in range(1, max_cycles + 1):
        run_result = DebugRun()
        if isinstance(run_result, str) and run_result.lower().startswith("error"):
            return {"error": run_result}

        deadline = time.time() + (max_wait_ms / 1000.0)
        while time.time() < deadline:
            if not IsDebugActive():
                break
            time.sleep(max(poll_interval_ms, 10) / 1000.0)
        else:
            return {"error": "Timed out waiting for break", "cycle": cycle, "last": last_info}

        rip = RegisterGet("RIP")
        if not isinstance(rip, str) or not rip.startswith("0x"):
            return {"error": "Failed to read RIP", "cycle": cycle, "last": last_info}

        base_info = MemoryBase(rip)
        base_addr = ""
        if isinstance(base_info, dict):
            base_addr = base_info.get("base_address", "")

        module_name = ""
        module_path = ""
        modules = GetModuleList()
        if isinstance(modules, list) and base_addr:
            for m in modules:
                if not isinstance(m, dict):
                    continue
                if m.get("base", "").lower() == base_addr.lower():
                    module_name = m.get("name", "")
                    module_path = m.get("path", "")
                    break

        last_info = {
            "rip": rip,
            "base": base_addr,
            "module": module_name,
            "path": module_path,
        }

        if not _is_system_module(module_name, module_path, system_root):
            return {
                "status": "user_code",
                "cycle": cycle,
                **last_info,
            }

    return {"error": "Max cycles reached without user code", "last": last_info}

@mcp.tool()
def CancelRunUntilUserCode() -> str:
    """
    Cancel an in-progress run-until-user-code operation

    Returns:
        Status message
    """
    return safe_get("Debug/CancelRunUntilUserCode")

# =============================================================================
# ASSEMBLER API
# =============================================================================

@mcp.tool()
def AssemblerAssemble(addr: str, instruction: str) -> dict:
    """
    Assemble instruction at address using Script API
    
    Parameters:
        addr: Memory address (in hex format, e.g. "0x1000")
        instruction: Assembly instruction (e.g. "mov eax, 1")
    
    Returns:
        Dictionary with assembly result
    """
    result = safe_get("Assembler/Assemble", {"addr": addr, "instruction": instruction})
    if isinstance(result, dict):
        return result
    elif isinstance(result, str):
        try:
            return json.loads(result)
        except:
            return {"error": "Failed to parse assembly result", "raw": result}
    return {"error": "Unexpected response format"}

@mcp.tool()
def AssemblerAssembleMem(addr: str, instruction: str, confirm: bool = False) -> str:
    """
    Assemble instruction directly into memory using Script API
    
    Parameters:
        addr: Memory address (in hex format, e.g. "0x1000")
        instruction: Assembly instruction (e.g. "mov eax, 1")
        confirm: Confirm write when safe mode is enabled
    
    Returns:
        Status message
    """
    params = {"addr": addr, "instruction": instruction}
    if confirm:
        params["confirm"] = "1"
    return safe_get("Assembler/AssembleMem", params)

# =============================================================================
# STACK API
# =============================================================================

@mcp.tool()
def StackPop(confirm: bool = False) -> str:
    """
    Pop value from stack using Script API
    
    Parameters:
        confirm: Confirm pop when safe mode is enabled

    Returns:
        Popped value in hex format
    """
    params = {}
    if confirm:
        params["confirm"] = "1"
    return safe_get("Stack/Pop", params)

@mcp.tool()
def StackPush(value: str, confirm: bool = False) -> str:
    """
    Push value to stack using Script API
    
    Parameters:
        value: Value to push (in hex format, e.g. "0x1000")
        confirm: Confirm push when safe mode is enabled
    
    Returns:
        Previous top value in hex format
    """
    params = {"value": value}
    if confirm:
        params["confirm"] = "1"
    return safe_get("Stack/Push", params)

@mcp.tool()
def StackPeek(offset: str = "0") -> str:
    """
    Peek at stack value using Script API
    
    Parameters:
        offset: Stack offset (default: "0")
    
    Returns:
        Stack value in hex format
    """
    return safe_get("Stack/Peek", {"offset": offset})

# =============================================================================
# FLAG API
# =============================================================================

@mcp.tool()
def FlagGet(flag: str) -> bool:
    """
    Get CPU flag value using Script API
    
    Parameters:
        flag: Flag name (ZF, OF, CF, PF, SF, TF, AF, DF, IF)
    
    Returns:
        Flag value (True/False)
    """
    result = safe_get("Flag/Get", {"flag": flag})
    if isinstance(result, str):
        return result.lower() == "true"
    return False

@mcp.tool()
def FlagSet(flag: str, value: bool, confirm: bool = False) -> str:
    """
    Set CPU flag value using Script API
    
    Parameters:
        flag: Flag name (ZF, OF, CF, PF, SF, TF, AF, DF, IF)
        value: Flag value (True/False)
        confirm: Confirm flag change when safe mode is enabled
    
    Returns:
        Status message
    """
    params = {"flag": flag, "value": "true" if value else "false"}
    if confirm:
        params["confirm"] = "1"
    return safe_get("Flag/Set", params)

# =============================================================================
# PATTERN API
# =============================================================================

@mcp.tool()
def PatternFindMem(start: str, size: str, pattern: str) -> str:
    """
    Find pattern in memory using Script API
    
    Parameters:
        start: Start address (in hex format, e.g. "0x1000")
        size: Size to search
        pattern: Pattern to find (e.g. "48 8B 05 ? ? ? ?")
    
    Returns:
        Found address in hex format or error message
    """
    return safe_get("Pattern/FindMem", {"start": start, "size": size, "pattern": pattern})

# =============================================================================
# MISC API
# =============================================================================

@mcp.tool()
def MiscParseExpression(expression: str) -> str:
    """
    Parse expression using Script API
    
    Parameters:
        expression: Expression to parse (e.g. "[esp+8]")
    
    Returns:
        Parsed value in hex format
    """
    return safe_get("Misc/ParseExpression", {"expression": expression})

@mcp.tool()
def MiscRemoteGetProcAddress(module: str, api: str) -> str:
    """
    Get remote procedure address using Script API
    
    Parameters:
        module: Module name (e.g. "kernel32.dll")
        api: API name (e.g. "GetProcAddress")
    
    Returns:
        Function address in hex format
    """
    return safe_get("Misc/RemoteGetProcAddress", {"module": module, "api": api})

# =============================================================================
# LEGACY COMPATIBILITY FUNCTIONS
# =============================================================================

@mcp.tool()
def SetRegister(name: str, value: str) -> str:
    """
    Set register value using command (legacy compatibility)
    
    Parameters:
        name: Register name (e.g. "eax", "rip")
        value: Value to set (in hex format, e.g. "0x1000")
    
    Returns:
        Status message
    """
    # Construct command to set register
    cmd = f"r {name}={value}"
    return ExecCommand(cmd)


@mcp.tool()
def DisasmGetInstruction(addr: str) -> dict:
    """
    Get disassembly of a single instruction at the specified address
    
    Parameters:
        addr: Memory address (in hex format, e.g. "0x1000")
    
    Returns:
        Dictionary containing instruction details
    """
    result = safe_get("Disasm/GetInstruction", {"addr": addr})
    if isinstance(result, dict):
        return result
    elif isinstance(result, str):
        try:
            return json.loads(result)
        except:
            return {"error": "Failed to parse disassembly result", "raw": result}
    return {"error": "Unexpected response format"}

@mcp.tool()
def DisasmGetInstructionRange(addr: str, count: int = 1) -> list:
    """
    Get disassembly of multiple instructions starting at the specified address
    
    Parameters:
        addr: Memory address (in hex format, e.g. "0x1000")
        count: Number of instructions to disassemble (default: 1, max: 100)
    
    Returns:
        List of dictionaries containing instruction details
    """
    result = safe_get("Disasm/GetInstructionRange", {"addr": addr, "count": str(count)})
    if isinstance(result, list):
        return result
    elif isinstance(result, str):
        try:
            return json.loads(result)
        except:
            return [{"error": "Failed to parse disassembly result", "raw": result}]
    return [{"error": "Unexpected response format"}]

@mcp.tool()
def DisasmGetInstructionAtRIP() -> dict:
    """
    Get disassembly of the instruction at the current RIP
    
    Returns:
        Dictionary containing current instruction details
    """
    result = safe_get("Disasm/GetInstructionAtRIP")
    if isinstance(result, dict):
        return result
    elif isinstance(result, str):
        try:
            return json.loads(result)
        except:
            return {"error": "Failed to parse disassembly result", "raw": result}
    return {"error": "Unexpected response format"}

@mcp.tool()
def StepInWithDisasm() -> dict:
    """
    Step into the next instruction and return both step result and current instruction disassembly
    
    Returns:
        Dictionary containing step result and current instruction info
    """
    result = safe_get("Disasm/StepInWithDisasm")
    if isinstance(result, dict):
        return result
    elif isinstance(result, str):
        try:
            return json.loads(result)
        except:
            return {"error": "Failed to parse step result", "raw": result}
    return {"error": "Unexpected response format"}


@mcp.tool()
def GetModuleList() -> list:
    """
    Get list of loaded modules
    
    Returns:
        List of module information (name, base address, size, etc.)
    """
    result = safe_get("GetModuleList")
    if isinstance(result, list):
        return result
    elif isinstance(result, str):
        try:
            return json.loads(result)
        except:
            return [{"error": "Failed to parse module list", "raw": result}]
    return [{"error": "Unexpected response format"}]

@mcp.tool()
def MemoryBase(addr: str) -> dict:
    """
    Find the base address and size of a module containing the given address
    
    Parameters:
        addr: Memory address (in hex format, e.g. "0x7FF12345")
    
    Returns:
        Dictionary containing base_address and size of the module
    """
    try:
        # Make the request to the endpoint
        result = safe_get("MemoryBase", {"addr": addr})
        
        # Handle different response types
        if isinstance(result, dict):
            return result
        elif isinstance(result, str):
            try:
                # Try to parse the string as JSON
                return json.loads(result)
            except:
                # Fall back to string parsing if needed
                if "," in result:
                    parts = result.split(",")
                    return {
                        "base_address": parts[0],
                        "size": parts[1]
                    }
                return {"raw_response": result}
        
        return {"error": "Unexpected response format"}
            
    except Exception as e:
        return {"error": str(e)}

import argparse

def main_cli():
    parser = argparse.ArgumentParser(description="x64dbg MCP CLI wrapper")

    parser.add_argument("tool", help="Tool/function name (e.g. ExecCommand, RegisterGet, MemoryRead)")
    parser.add_argument("args", nargs="*", help="Arguments for the tool")
    parser.add_argument("--x64dbg-url", dest="x64dbg_url", default=os.getenv("X64DBG_URL"), help="x64dbg HTTP server URL")

    opts = parser.parse_args()

    if opts.x64dbg_url:
        set_x64dbg_server_url(opts.x64dbg_url)

    # Map CLI call → actual MCP tool function
    if opts.tool in globals():
        func = globals()[opts.tool]
        if callable(func):
            try:
                # Try to unpack args dynamically
                result = func(*opts.args)
                print(json.dumps(result, indent=2))
            except TypeError as e:
                print(f"Error calling {opts.tool}: {e}")
        else:
            print(f"{opts.tool} is not callable")
    else:
        print(f"Unknown tool: {opts.tool}")


def claude_cli():
    parser = argparse.ArgumentParser(description="Chat with Claude using x64dbg MCP tools")
    parser.add_argument("prompt", nargs=argparse.REMAINDER, help="Initial user prompt. If empty, read from stdin")
    parser.add_argument("--model", dest="model", default=os.getenv("ANTHROPIC_MODEL", "claude-3-7-sonnet-2025-06-20"), help="Claude model")
    parser.add_argument("--api-key", dest="api_key", default=os.getenv("ANTHROPIC_API_KEY"), help="Anthropic API key")
    parser.add_argument("--system", dest="system", default="You can control x64dbg via MCP tools.", help="System prompt")
    parser.add_argument("--max-steps", dest="max_steps", type=int, default=100, help="Max tool-use iterations")
    parser.add_argument("--x64dbg-url", dest="x64dbg_url", default=os.getenv("X64DBG_URL"), help="x64dbg HTTP server URL")
    parser.add_argument("--no-tools", dest="no_tools", action="store_true", help="Disable tool-use (text-only)")

    opts = parser.parse_args()

    if opts.x64dbg_url:
        set_x64dbg_server_url(opts.x64dbg_url)

    # Resolve prompt
    user_prompt = " ".join(opts.prompt).strip()
    if not user_prompt:
        user_prompt = sys.stdin.read().strip()
    if not user_prompt:
        print("No prompt provided.")
        return

    try:
        import anthropic
    except Exception as e:
        print("Anthropic SDK not installed. Run: pip install anthropic")
        print(str(e))
        return

    if not opts.api_key:
        print("Missing Anthropic API key. Set ANTHROPIC_API_KEY or pass --api-key.")
        return

    client = anthropic.Anthropic(api_key=opts.api_key)

    tools_spec: List[Dict[str, Any]] = []
    if not opts.no_tools:
        tools_spec = [
            {
                "name": "mcp_list_tools",
                "description": "List available MCP tool functions and their parameters.",
                "input_schema": {"type": "object", "properties": {}},
            },
            {
                "name": "mcp_call_tool",
                "description": "Invoke an MCP tool by name with arguments.",
                "input_schema": {
                    "type": "object",
                    "properties": {
                        "tool": {"type": "string"},
                        "args": {"type": "object"}
                    },
                    "required": ["tool"],
                },
            },
        ]

    messages: List[Dict[str, Any]] = [
        {"role": "user", "content": user_prompt}
    ]

    step = 0
    while True:
        step += 1
        response = client.messages.create(
            model=opts.model,
            system=opts.system,
            messages=messages,
            tools=tools_spec if not opts.no_tools else None,
            max_tokens=1024,
        )

        # Print any assistant text
        assistant_text_chunks: List[str] = []
        tool_uses: List[Dict[str, Any]] = []
        for block in response.content:
            b = _block_to_dict(block)
            if b.get("type") == "text":
                assistant_text_chunks.append(b.get("text", ""))
            elif b.get("type") == "tool_use":
                tool_uses.append(b)

        if assistant_text_chunks:
            print("\n".join(assistant_text_chunks))

        if not tool_uses or opts.no_tools:
            break

        # Prepare tool results as a new user message
        tool_result_blocks: List[Dict[str, Any]] = []
        for tu in tool_uses:
            name = tu.get("name")
            tu_id = tu.get("id")
            input_obj = tu.get("input", {}) or {}
            result: Any
            if name == "mcp_list_tools":
                result = {"tools": _list_tools_description()}
            elif name == "mcp_call_tool":
                tool_name = input_obj.get("tool")
                args = input_obj.get("args", {}) or {}
                result = _invoke_tool_by_name(tool_name, args)
            else:
                result = {"error": f"Unknown tool: {name}"}

            # Ensure serializable content (string)
            try:
                result_text = json.dumps(result)
            except Exception:
                result_text = str(result)

            tool_result_blocks.append({
                "type": "tool_result",
                "tool_use_id": tu_id,
                "content": result_text,
            })

        # Normalize assistant content to plain dicts
        assistant_blocks = [_block_to_dict(b) for b in response.content]
        messages.append({"role": "assistant", "content": assistant_blocks})
        messages.append({"role": "user", "content": tool_result_blocks})

        if step >= opts.max_steps:
            break

if __name__ == "__main__":
    # Support multiple modes:
    #  - "serve" or "--serve": run MCP server
    #  - "claude" subcommand: run Claude Messages chat loop
    #  - default: tool invocation CLI
    if len(sys.argv) > 1:
        if sys.argv[1] in ("--serve", "serve"):
            mcp.run()
        elif sys.argv[1] == "claude":
            # Shift off the subcommand and re-dispatch
            sys.argv.pop(1)
            claude_cli()
        else:
            main_cli()
    else:
        mcp.run()

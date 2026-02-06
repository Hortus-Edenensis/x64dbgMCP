import pathlib
import sys
import unittest
import types
from unittest import mock


ROOT = pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "src"))


if "requests" not in sys.modules:
    requests_stub = types.ModuleType("requests")
    requests_stub.get = lambda *args, **kwargs: None
    requests_stub.post = lambda *args, **kwargs: None
    sys.modules["requests"] = requests_stub

if "mcp.server.fastmcp" not in sys.modules:
    fastmcp_module = types.ModuleType("mcp.server.fastmcp")

    class _DummyFastMCP:
        def __init__(self, *args, **kwargs):
            pass

        def tool(self):
            def decorator(func):
                return func
            return decorator

        def run(self):
            return None

    fastmcp_module.FastMCP = _DummyFastMCP
    server_module = types.ModuleType("mcp.server")
    server_module.fastmcp = fastmcp_module
    mcp_module = types.ModuleType("mcp")
    mcp_module.server = server_module
    sys.modules["mcp"] = mcp_module
    sys.modules["mcp.server"] = server_module
    sys.modules["mcp.server.fastmcp"] = fastmcp_module

import x64dbg  # noqa: E402


class X64DbgPythonWrapperTests(unittest.TestCase):
    def test_exec_command_requires_confirm_for_dangerous_command(self):
        with mock.patch.object(x64dbg, "safe_get") as safe_get_mock:
            out = x64dbg.ExecCommand("run", wait=False, confirm=False)
        self.assertIn("requires elevation confirmation", out.lower())
        safe_get_mock.assert_not_called()

    def test_exec_command_whitelist_allows_without_confirm(self):
        with mock.patch.object(x64dbg, "safe_get", return_value="ok") as safe_get_mock:
            out = x64dbg.ExecCommand("help", wait=False, confirm=False)
        self.assertEqual("ok", out)
        safe_get_mock.assert_called_once()

    def test_set_register_calls_register_set(self):
        with mock.patch.object(x64dbg, "RegisterSet", return_value="done") as register_set_mock:
            with mock.patch.object(x64dbg, "ExecCommand") as exec_command_mock:
                out = x64dbg.SetRegister("eax", "0x1234", confirm=True)
        self.assertEqual("done", out)
        register_set_mock.assert_called_once_with(register="eax", value="0x1234", confirm=True)
        exec_command_mock.assert_not_called()


if __name__ == "__main__":
    unittest.main()

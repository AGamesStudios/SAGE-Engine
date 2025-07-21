"""Execute Python scripts in a restricted environment."""
from __future__ import annotations

import builtins
import sys
from pathlib import Path
from typing import Any, Dict

_allowed_modules = {"math", "random"}
_globals: Dict[str, Any] = {}


def set_python_globals(**values: Any) -> None:
    """Expose objects to Python scripts."""
    _globals.update(values)


def _safe_import(name: str, globals=None, locals=None, fromlist=(), level=0):
    root = name.split(".")[0]
    if root not in _allowed_modules:
        raise ImportError(f"Module '{root}' not allowed")
    return __import__(name, globals, locals, fromlist, level)


_SAFE_BUILTINS = {k: getattr(builtins, k) for k in [
    "abs",
    "min",
    "max",
    "range",
    "len",
    "print",
    "float",
    "int",
    "str",
    "bool",
    "enumerate",
    "list",
    "dict",
    "set",
    "tuple",
    "zip",
]}
_SAFE_BUILTINS["__import__"] = _safe_import


def run_python_script(path: str) -> None:
    """Execute a Python script file in a restricted namespace."""
    folder = str(Path(path).resolve().parent)
    if folder not in sys.path:
        sys.path.insert(0, folder)
    source = Path(path).read_text(encoding="utf-8")
    env: Dict[str, Any] = {"__builtins__": _SAFE_BUILTINS}
    env.update(_globals)
    exec(compile(source, path, "exec"), env, env)


__all__ = ["run_python_script", "set_python_globals"]

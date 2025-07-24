"""Execute Python scripts in a restricted environment."""
from __future__ import annotations

import ast
import builtins
import sys
from pathlib import Path
from typing import Any, Dict

from sage.config import load_config
from .safeimport import (
    DEFAULT_ALLOWED_MODULES,
    BLOCKED_MODULES,
    validate_imports,
)

_globals: Dict[str, Any] = {}


def _allowed_modules() -> set[str]:
    cfg = load_config()
    custom = cfg.get("allowed_modules", [])
    return set(DEFAULT_ALLOWED_MODULES).union(custom)


def _safe_import(name: str, globals=None, locals=None, fromlist=(), level=0):
    allowed = _allowed_modules()
    root = name.split(".")[0]
    if name in BLOCKED_MODULES or root in BLOCKED_MODULES:
        raise ImportError(f"Module '{root}' not allowed")
    if name not in allowed and root not in allowed:
        raise ImportError(f"Module '{root}' not allowed")
    return __import__(name, globals, locals, fromlist, level)


def set_python_globals(**values: Any) -> None:
    """Expose objects to Python scripts."""
    _globals.update(values)


def _validate_ast(tree: ast.AST) -> None:
    """Validate parsed AST for unsafe constructs."""
    validate_imports(tree, _allowed_modules(), BLOCKED_MODULES)
    for parent in ast.walk(tree):
        for child in ast.iter_child_nodes(parent):
            setattr(child, "parent", parent)
    for node in ast.walk(tree):
        if isinstance(node, ast.Call):
            if isinstance(node.func, ast.Name) and node.func.id in {
                "eval",
                "exec",
                "compile",
                "open",
                "input",
                "__import__",
            }:
                raise RuntimeError("Forbidden function usage")
        elif isinstance(node, ast.Name):
            if node.id in {"globals", "locals"}:
                raise RuntimeError("Forbidden name usage")
        elif isinstance(node, ast.Lambda):
            raise RuntimeError("lambda expressions not allowed")
        elif isinstance(node, ast.While):
            if isinstance(node.test, ast.Constant) and node.test.value is True:
                raise RuntimeError("potential infinite loop")
        elif isinstance(node, ast.ClassDef):
            for kw in node.keywords:
                if kw.arg == "metaclass":
                    raise RuntimeError("metaclass definition not allowed")
        elif isinstance(node, ast.FunctionDef):
            parent = getattr(node, "parent", None)
            if parent and not isinstance(parent, ast.Module):
                raise RuntimeError("nested functions not allowed")


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
for name in [
    "open",
    "exec",
    "eval",
    "compile",
    "input",
    "exit",
    "globals",
    "locals",
]:
    _SAFE_BUILTINS[name] = None


def run_python_script(path: str) -> None:
    """Execute a Python script file in a restricted namespace."""
    folder = str(Path(path).resolve().parent)
    if folder not in sys.path:
        sys.path.insert(0, folder)
    source = Path(path).read_text(encoding="utf-8")
    if source.count("\n") > 1000:
        raise RuntimeError("script too long")
    tree = ast.parse(source, path)
    _validate_ast(tree)
    env: Dict[str, Any] = {"__builtins__": _SAFE_BUILTINS}
    pre: Dict[str, Any] = {}
    for mod in _allowed_modules():
        if mod == "builtins":
            continue
        try:
            pre[mod] = __import__(mod)
        except ImportError:
            pass
    env.update(pre)
    env.update(_globals)
    exec(compile(tree, path, "exec"), env, env)


__all__ = ["run_python_script", "set_python_globals"]

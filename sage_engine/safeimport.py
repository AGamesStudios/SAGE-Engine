"""Validate imports used within Python scripts."""
from __future__ import annotations

import ast
from typing import Iterable, Set

DEFAULT_ALLOWED_MODULES: Set[str] = {
    "math",
    "random",
    "time",
    "datetime",
    "logic_api",
    "sage_engine.logic_api",
}

BLOCKED_MODULES: Set[str] = {
    "os",
    "sys",
    "socket",
    "subprocess",
    "threading",
    "multiprocessing",
    "ctypes",
    "platform",
    "builtins",
    "inspect",
    "importlib",
}


def validate_imports(
    tree: ast.AST,
    allowed: Iterable[str] | None = None,
    blocked: Iterable[str] | None = None,
) -> None:
    """Raise ImportError if an import is outside the allowlist."""
    allowed_set = set(DEFAULT_ALLOWED_MODULES)
    if allowed is not None:
        allowed_set.update(allowed)
    blocked_set = set(BLOCKED_MODULES)
    if blocked is not None:
        blocked_set.update(blocked)

    for node in ast.walk(tree):
        if isinstance(node, ast.Import):
            for alias in node.names:
                name = alias.name
                root = name.split(".")[0]
                if name in blocked_set or root in blocked_set:
                    raise ImportError(f"Module '{root}' is not allowed in sandbox")
                if name not in allowed_set and root not in allowed_set:
                    raise ImportError(f"Module '{root}' is not allowed in sandbox")
        elif isinstance(node, ast.ImportFrom):
            module = node.module or ""
            root = module.split(".")[0]
            if module in blocked_set or root in blocked_set:
                raise ImportError(f"Module '{root}' is not allowed in sandbox")
            if module not in allowed_set and root not in allowed_set:
                raise ImportError(f"Module '{root}' is not allowed in sandbox")

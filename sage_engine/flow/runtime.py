from __future__ import annotations

from types import SimpleNamespace
from typing import Any
from .bindings import get_builtins

__all__ = ["FlowRuntime", "run"]

class FlowRuntime:
    """Minimal FlowScript interpreter supporting Python-like snippets."""

    def __init__(self) -> None:
        self.globals: dict[str, Any] = {}
        self.globals.update(get_builtins())

    async def run(self, script: str, context: dict[str, Any] | None = None, *, dialect: str = "python") -> None:
        """Execute a script in the provided context."""
        ctx = {} if context is None else context
        # merge builtins
        ctx = {**self.globals, **ctx}
        if dialect == "python":
            exec(script, ctx)
        elif dialect == "flow":
            from .compiler import compile_ast
            from .parser import parse
            ast = parse(script)
            code = compile_ast(ast)
            exec(code, ctx)
        else:
            raise ValueError(f"Unknown dialect: {dialect}")


def run(script: str, context: dict[str, Any]) -> Any:
    """Compatibility helper used by tests."""
    rt = FlowRuntime()
    return rt.run(script, context)

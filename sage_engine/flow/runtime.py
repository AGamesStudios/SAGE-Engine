from __future__ import annotations

from typing import Any

from .bindings import get_builtins

from .compiler import compile_source, decode_bytes
from .bytecode.vm import run as run_code

__all__ = ["FlowRuntime", "run", "run_bytecode"]


class FlowRuntime:
    """Simple FlowScript interpreter supporting basic Flow language."""

    def __init__(self) -> None:
        self.globals: dict[str, Any] = get_builtins()

    def _exec(self, code: Any, ctx: dict[str, Any]) -> None:
        run_code(code, ctx)

    async def run(
        self, script: str, context: dict[str, Any] | None = None, *, dialect: str = "python"
    ) -> None:
        """Execute a script using the given dialect."""
        ctx = {} if context is None else context
        exec_ctx = {**self.globals, **ctx}
        if dialect == "python":
            code = compile(script, "<python>", "exec")
        elif dialect in {"flow", "ru", "en"}:
            code = compile_source(script, lang="ru" if dialect == "flow" else dialect)
        else:
            raise ValueError(f"Unknown dialect: {dialect}")
        self._exec(code, exec_ctx)
        ctx.update({k: v for k, v in exec_ctx.items() if k not in self.globals})

    def run_bytecode(self, data: bytes, context: dict[str, Any] | None = None) -> None:
        ctx = {} if context is None else context
        exec_ctx = {**self.globals, **ctx}
        code = decode_bytes(data)
        self._exec(code, exec_ctx)
        ctx.update({k: v for k, v in exec_ctx.items() if k not in self.globals})


def run(script: str, context: dict[str, Any]) -> Any:
    rt = FlowRuntime()
    return rt.run(script, context)

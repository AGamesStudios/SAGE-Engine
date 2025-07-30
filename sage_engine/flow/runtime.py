from __future__ import annotations

from typing import Any
from pathlib import Path

from .bindings import get_builtins
from ..input import Input
from ..input.state import KEY_MAP
from ..logger import logger


class _FlowInputProxy:
    """Proxy around Input that logs errors with [flow] prefix."""

    def _validate(self, key: str) -> str | None:
        if Input.is_action_bound(key) or key.upper() in KEY_MAP:
            return None
        import difflib
        candidates = list(KEY_MAP.keys()) + list(Input._actions.keys())  # type: ignore[attr-defined]
        match = difflib.get_close_matches(key.upper(), candidates, n=1)
        hint = match[0] if match else None
        logger.error("[flow] Неизвестная клавиша или действие: '%s'", key)
        if hint:
            logger.info("[HINT] [flow] Возможно вы имели в виду '%s'", hint)
        return hint

    def is_pressed(self, key: str) -> bool:
        if self._validate(key):
            return False
        return Input.is_pressed(key)

    def is_down(self, key: str) -> bool:
        if self._validate(key):
            return False
        return Input.is_down(key)

    def is_up(self, key: str) -> bool:
        if self._validate(key):
            return False
        return Input.is_up(key)

    def __getattr__(self, name: str) -> Any:  # fallback to Input
        return getattr(Input, name)

import ast
import asyncio

from .compiler import compile_source, decode_bytes
from .bytecode.vm import run as run_code

__all__ = ["FlowRuntime", "run", "run_bytecode", "run_flow_script"]


class FlowRuntime:
    """Simple FlowScript interpreter supporting basic Flow language."""

    def __init__(self) -> None:
        self.globals: dict[str, Any] = get_builtins()
        self.globals["Input"] = _FlowInputProxy()

    def _exec(self, code: ast.Module, ctx: dict[str, Any]) -> None:
        run_code(code, ctx)

    async def run(
        self, script: str, context: dict[str, Any] | None = None, *, dialect: str = "python"
    ) -> None:
        """Execute a script using the given dialect."""
        ctx = {} if context is None else context
        exec_ctx = {**self.globals, **ctx}
        if dialect == "python":
            code = ast.parse(script, "<python>")
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


_CACHE: dict[str, tuple[Any, dict[str, Any]]] = {}


def run_flow_script(path: str, input_obj: Any, *, lang: str = "ru") -> dict[str, Any]:
    """Execute a FlowScript file and invoke its on_update handler."""
    if path not in _CACHE:
        text = Path(path).read_text(encoding="utf-8")
        ctx: dict[str, Any] = {"Input": _FlowInputProxy()}
        code = compile_source(text, lang=lang)
        run_code(code, ctx)
        _CACHE[path] = (code, ctx)
    code, ctx = _CACHE[path]
    ctx["Input"] = _FlowInputProxy()
    fn = ctx.get("on_update")
    if fn:
        asyncio.run(fn())
    return ctx

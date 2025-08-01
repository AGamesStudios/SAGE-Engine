"""Simple tool activation API."""
from __future__ import annotations

_active = None
_tools = {}


class BaseTool:
    def on_mouse(self, *a, **k):
        pass

    def on_key(self, *a, **k):
        pass


def register_tool(name: str, tool: BaseTool) -> None:
    _tools[name] = tool


def set_active(name: str) -> None:
    global _active
    _active = _tools.get(name)


def get_active() -> BaseTool | None:
    return _active

"""Lua-like dialect placeholder."""

__all__ = ["compile"]


def compile(script: str) -> str:
    # Very naive conversion: replace 'then'/'end'
    py = script.replace("then", ":").replace("end", "")
    return py

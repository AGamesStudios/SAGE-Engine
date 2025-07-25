"""SAGE Terminal package.

The terminal uses :mod:`customtkinter` which may not be available in every
environment. To avoid import errors when the package is used only for its
command helpers, access ``main`` or ``TerminalApp`` lazily through
``__getattr__``.
"""

from __future__ import annotations

__all__ = ["main", "TerminalApp"]


def __getattr__(name: str):  # pragma: no cover - thin loader
    if name in {"main", "TerminalApp"}:
        from .terminal import main, TerminalApp
        globals().update(main=main, TerminalApp=TerminalApp)
        return globals()[name]
    raise AttributeError(name)


"""Shader and visual effects stubs."""

from __future__ import annotations


class Effect:
    def __init__(self, name: str) -> None:
        self.name = name

    def apply(self):  # pragma: no cover - placeholder
        pass


__all__ = ["Effect"]

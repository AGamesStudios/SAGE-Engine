"""Wrapper utilities for GUI visual effects."""
from __future__ import annotations

from typing import Callable

from . import fx

EffectFunc = Callable[[bytearray, int, int], None]


def register_effect(name: str, func: EffectFunc) -> None:
    fx.register(name, func)


def apply_effect(name: str, buffer: bytearray, width: int, height: int) -> None:
    fx.apply(name, buffer, width, height)

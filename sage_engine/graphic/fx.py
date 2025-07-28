"""Simple registry of framebuffer effects."""
from __future__ import annotations

from typing import Callable, Dict

EffectFunc = Callable[[bytearray, int, int], None]

_registry: Dict[str, EffectFunc] = {}


def register(name: str, func: EffectFunc) -> None:
    _registry[name] = func


def apply(name: str, buffer: bytearray, width: int, height: int) -> None:
    effect = _registry.get(name)
    if effect is not None:
        effect(buffer, width, height)


def list_effects():
    return list(_registry.keys())


def blur_effect(buffer: bytearray, width: int, height: int) -> None:
    """Naive box blur."""
    pitch = width * 4
    src = buffer[:]
    for y in range(height):
        for x in range(width):
            r = g = b = a = count = 0
            for yy in range(max(0, y - 1), min(height, y + 2)):
                for xx in range(max(0, x - 1), min(width, x + 2)):
                    off = yy * pitch + xx * 4
                    b += src[off]
                    g += src[off + 1]
                    r += src[off + 2]
                    a += src[off + 3]
                    count += 1
            off = y * pitch + x * 4
            buffer[off] = b // count
            buffer[off + 1] = g // count
            buffer[off + 2] = r // count
            buffer[off + 3] = a // count

register("blur", blur_effect)

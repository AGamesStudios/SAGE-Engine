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


def edge_average_2x(buffer: bytearray, width: int, height: int) -> None:
    pitch = width * 4
    src = buffer[:]
    for y in range(height):
        for x in range(width):
            off = y * pitch + x * 4
            total = [0, 0, 0, 0]
            count = 0
            for yy in (y, min(y + 1, height - 1)):
                for xx in (x, min(x + 1, width - 1)):
                    o2 = yy * pitch + xx * 4
                    for i in range(4):
                        total[i] += src[o2 + i]
                    count += 1
            for i in range(4):
                buffer[off + i] = total[i] // count


def fast_blur_4x(buffer: bytearray, width: int, height: int) -> None:
    edge_average_2x(buffer, width, height)
    edge_average_2x(buffer, width, height)


def smart_subpixel_8x(buffer: bytearray, width: int, height: int) -> None:
    fast_blur_4x(buffer, width, height)


def ai_emulated_16x(buffer: bytearray, width: int, height: int) -> None:
    smart_subpixel_8x(buffer, width, height)


register("EdgeAverage2x", edge_average_2x)
register("FastBlur4x", fast_blur_4x)
register("SmartSubpixel8x", smart_subpixel_8x)
register("AIEmulated16x", ai_emulated_16x)


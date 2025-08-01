"""Fixed-point and blending helpers for software renderer."""

from __future__ import annotations

Q8_SHIFT = 8
Q8_ONE = 1 << Q8_SHIFT


def to_fixed(value: float) -> int:
    """Convert ``value`` to Q8.8 fixed point."""
    return int(value * Q8_ONE)


def round_fixed(value: int) -> int:
    """Round a Q8.8 value to ``int``."""
    return (value + (Q8_ONE // 2)) >> Q8_SHIFT

def mul_fixed(a: int, b: int) -> int:
    """Multiply two Q8.8 values and return Q8.8."""
    return (a * b) >> Q8_SHIFT


def div_fixed(a: int, b: int) -> int:
    """Divide ``a`` by ``b`` assuming Q8.8 numbers."""
    if b == 0:
        return 0
    return (a << Q8_SHIFT) // b


def q8_mul(a: int, b: int) -> int:
    return mul_fixed(a, b)


def q8_lerp(a: int, b: int, t: int) -> int:
    """Linear interpolation between Q8.8 numbers ``a`` and ``b`` using 0..255 ``t``."""
    return a + (((b - a) * t) >> 8)


def blend_rgba_pm(dst: int, src: int) -> int:
    """Blend two premultiplied RGBA8888 pixels."""
    da = (dst >> 24) & 0xFF
    sa = (src >> 24) & 0xFF
    inv = 255 - sa
    dr = (dst >> 16) & 0xFF
    dg = (dst >> 8) & 0xFF
    db = dst & 0xFF
    sr = (src >> 16) & 0xFF
    sg = (src >> 8) & 0xFF
    sb = src & 0xFF
    r = sr + ((dr * inv) >> 8)
    g = sg + ((dg * inv) >> 8)
    b = sb + ((db * inv) >> 8)
    a = sa + ((da * inv) >> 8)
    return (r << 16) | (g << 8) | b | (a << 24)

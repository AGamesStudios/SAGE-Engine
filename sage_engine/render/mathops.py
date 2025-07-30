"""Fixed-point and blending helpers for software renderer."""

from __future__ import annotations

# Q8.8 fixed point multiply and lerp

def q8_mul(a: int, b: int) -> int:
    """Multiply two Q8.8 values and return Q8.8."""
    return (a * b) >> 8

def q8_lerp(a: int, b: int, t: int) -> int:
    """Linear interpolation between Q8.8 numbers a and b using 0..255 t."""
    return ((a * (255 - t)) + (b * t)) >> 8

def blend_rgba_pm(dst: int, src: int) -> int:
    """Blend two premultiplied RGBA8888 pixels."""
    sa = (src >> 24) & 0xFF
    inv = 255 - sa
    r = (src & 0xFF) + (((dst & 0xFF) * inv) >> 8)
    g = ((src >> 8) & 0xFF) + ((((dst >> 8) & 0xFF) * inv) >> 8)
    b = ((src >> 16) & 0xFF) + ((((dst >> 16) & 0xFF) * inv) >> 8)
    a = sa + (((dst >> 24) & 0xFF) * inv >> 8)
    return (a << 24) | (b << 16) | (g << 8) | r

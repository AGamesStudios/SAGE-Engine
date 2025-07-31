"""Fixed-point and blending helpers for software renderer."""

from __future__ import annotations

from . import rustbridge as rust


def q8_mul(a: int, b: int) -> int:
    """Multiply two Q8.8 values and return Q8.8 using Rust helper."""
    return int(rust.lib.sage_q8_mul(a, b))


def q8_lerp(a: int, b: int, t: int) -> int:
    """Linear interpolation between Q8.8 numbers ``a`` and ``b`` using 0..255 ``t``."""
    return int(rust.lib.sage_q8_lerp(a, b, t))


def blend_rgba_pm(dst: int, src: int) -> int:
    """Blend two premultiplied RGBA8888 pixels using Rust implementation."""
    return int(rust.lib.sage_blend_rgba_pm(dst, src))

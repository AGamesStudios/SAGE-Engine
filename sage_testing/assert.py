"""Additional assertions used by SAGE Testing."""
from __future__ import annotations


def assert_approx_equal(a: float, b: float, *, eps: float = 1e-6) -> None:
    if abs(a - b) > eps:
        raise AssertionError(f"{a} != {b} (eps={eps})")

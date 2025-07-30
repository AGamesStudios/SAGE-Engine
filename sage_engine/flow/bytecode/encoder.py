from __future__ import annotations

__all__ = ["encode"]


def encode(src: str):
    """Compile Python source produced by the parser into a code object."""
    return compile(src, "<flowscript>", "exec")

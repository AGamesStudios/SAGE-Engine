from __future__ import annotations

import ast

__all__ = ["encode"]


def encode(src: str) -> ast.Module:
    """Parse FlowScript-translated Python source into an AST module."""
    return ast.parse(src, "<flowscript>")

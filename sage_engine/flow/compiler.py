from __future__ import annotations

from typing import List

__all__ = ["compile_ast"]


def compile_ast(ast: List[str]) -> str:
    """Very naive compiler converting token list back to Python code."""
    return " ".join(ast)

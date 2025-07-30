"""Very small FlowScript dialect parser."""

from ..parser import parse
from ..compiler import compile_ast

__all__ = ["compile"]


def compile(script: str) -> str:
    tokens = parse(script)
    return compile_ast(tokens)

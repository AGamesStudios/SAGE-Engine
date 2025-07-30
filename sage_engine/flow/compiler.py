from __future__ import annotations

from typing import Any
import ast

from .grammar import parser
from .bytecode import encoder

__all__ = ["compile_source", "compile_to_bytes", "decode_bytes"]


def compile_source(script: str, *, lang: str = "ru") -> ast.Module:
    """Compile FlowScript text to an AST object."""
    source = parser.parse(script, lang=lang)
    return encoder.encode(source)


def compile_to_bytes(script: str, *, lang: str = "ru") -> bytes:
    """Compile FlowScript text and return serialized source bytes."""
    source = parser.parse(script, lang=lang)
    return source.encode("utf-8")


def decode_bytes(data: bytes) -> ast.Module:
    """Decode serialized source bytes into an AST object."""
    source = data.decode("utf-8")
    return encoder.encode(source)

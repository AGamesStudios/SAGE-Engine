from __future__ import annotations

from typing import Any
import marshal

from .grammar import parser
from .bytecode import encoder

__all__ = ["compile_source", "compile_to_bytes", "decode_bytes"]


def compile_source(script: str, *, lang: str = "ru") -> Any:
    """Compile FlowScript text to a Python code object."""
    source = parser.parse(script, lang=lang)
    code = encoder.encode(source)
    return code


def compile_to_bytes(script: str, *, lang: str = "ru") -> bytes:
    """Compile FlowScript text and return serialized bytecode."""
    code = compile_source(script, lang=lang)
    return marshal.dumps(code)


def decode_bytes(data: bytes) -> Any:
    """Decode serialized FlowScript bytecode into a code object."""
    return marshal.loads(data)

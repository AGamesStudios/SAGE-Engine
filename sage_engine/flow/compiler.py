from __future__ import annotations

from typing import Any
from .grammar import parser
from .bytecode import encoder

__all__ = ["compile_source"]


def compile_source(script: str, *, lang: str = "ru") -> Any:
    """Compile FlowScript text to a Python code object."""
    source = parser.parse(script, lang=lang)
    code = encoder.encode(source)
    return code

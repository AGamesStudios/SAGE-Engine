from __future__ import annotations

from typing import List

__all__ = ["parse"]


def parse(script: str) -> List[str]:
    """Trivial parser splitting script into tokens by whitespace."""
    tokens = script.replace(";", " ").split()
    return tokens

from __future__ import annotations

__all__ = ["tokenize"]


def tokenize(text: str) -> list[str]:
    """Split text into lines preserving indentation."""
    return [line.rstrip() for line in text.splitlines() if line.strip()]

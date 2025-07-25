"""Utility helpers for SAGE Terminal."""
from __future__ import annotations

import os


def format_success(message: str) -> str:
    return f"[\u2713] {message}"


def format_error(message: str) -> str:
    return f"[\u2717] {message}"


def list_dir(path: str) -> str:
    lines = []
    for root, dirs, files in os.walk(path):
        indent = " " * (root.count(os.sep) - path.count(os.sep))
        for d in dirs:
            lines.append(f"{indent}{d}/")
        for f in files:
            lines.append(f"{indent}{f}")
    return "\n".join(lines)

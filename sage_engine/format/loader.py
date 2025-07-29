from __future__ import annotations

"""Centralized helpers for loading SAGE binary files."""

from pathlib import Path
from . import SAGEDecompiler


def load_sage_file(path: str | Path):
    """Load a `.sage*` file and return the decoded data."""
    return SAGEDecompiler().decompile(Path(path))

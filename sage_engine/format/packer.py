from __future__ import annotations

"""Simple helper to pack multiple `.sage*` files into one archive."""

from pathlib import Path
import zipfile


def pack_directory(src: Path, dst: Path) -> None:
    """Create a zip archive containing all `.sage*` files."""
    with zipfile.ZipFile(dst, "w") as zf:
        for path in src.rglob("*.sage*"):
            zf.write(path, path.relative_to(src))

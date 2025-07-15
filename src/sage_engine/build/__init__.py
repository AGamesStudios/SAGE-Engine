from __future__ import annotations

import hashlib
import os
from pathlib import Path

from ..soundmint import convert as _convert


def sha1(path: str | os.PathLike[str]) -> str:
    """Return SHA-1 hex digest of a file."""
    h = hashlib.sha1()
    with open(path, "rb") as f:
        for chunk in iter(lambda: f.read(8192), b""):
            h.update(chunk)
    return h.hexdigest()


def convert_audio(path: str, cache_dir: str) -> Path:
    """Convert *path* to the opposite format using :mod:`soundmint`."""

    ext = Path(path).suffix.lower()
    if ext == ".ogg":
        target_fmt = "mp3"
    elif ext in {".mp3", ".wav"}:
        target_fmt = "ogg"
    else:
        raise ValueError("unsupported format")

    os.makedirs(cache_dir, exist_ok=True)
    digest = sha1(path)
    dest = Path(cache_dir) / f"{digest}.{target_fmt}"

    if dest.exists() and dest.stat().st_mtime >= Path(path).stat().st_mtime:
        return dest

    converted = _convert(path, target_fmt)
    if converted != dest:
        os.replace(converted, dest)
    return dest


__all__ = ["convert_audio", "sha1"]

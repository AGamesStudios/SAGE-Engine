from __future__ import annotations

import hashlib
import os
import shutil
from pathlib import Path


def sha1(path: str | os.PathLike[str]) -> str:
    """Return SHA-1 hex digest of a file."""
    h = hashlib.sha1()
    with open(path, "rb") as f:
        for chunk in iter(lambda: f.read(8192), b""):
            h.update(chunk)
    return h.hexdigest()


def convert_audio(path: str, cache_dir: str) -> Path:
    """Copy an OGG or MP3 file into *cache_dir* using its SHA-1 name."""
    ext = Path(path).suffix.lower()
    if ext not in {".ogg", ".mp3"}:
        raise ValueError("unsupported format")
    os.makedirs(cache_dir, exist_ok=True)
    digest = sha1(path)
    target_ext = ".mp3" if ext == ".ogg" else ".ogg"
    dest = Path(cache_dir) / f"{digest}{target_ext}"
    if not dest.exists():
        shutil.copy2(path, dest)
    return dest


__all__ = ["convert_audio", "sha1"]

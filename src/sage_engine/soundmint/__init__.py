"""Simple audio converter with SHA-1 caching."""

from __future__ import annotations

import hashlib
import os
from pathlib import Path

from pydub import AudioSegment


def _sha1(path: Path) -> str:
    h = hashlib.sha1()
    with open(path, "rb") as f:
        for chunk in iter(lambda: f.read(8192), b""):
            h.update(chunk)
    return h.hexdigest()


def convert(src_path: str, target_fmt: str) -> Path:
    """Convert *src_path* to *target_fmt* using FFmpeg.

    Only ``ogg`` -> ``mp3`` and ``wav``/``mp3`` -> ``ogg`` are supported.
    Returns the cached file path under ``build/audio_cache``.
    """

    src = Path(src_path)
    ext = src.suffix.lower().lstrip(".")
    if (ext, target_fmt) not in {("ogg", "mp3"), ("wav", "ogg"), ("mp3", "ogg")}:
        raise ValueError("unsupported conversion")

    digest = _sha1(src)
    cache_dir = Path("build/audio_cache")
    cache_dir.mkdir(parents=True, exist_ok=True)
    dest = (cache_dir / f"{digest}.{target_fmt}").resolve()

    if dest.exists() and dest.stat().st_mtime >= src.stat().st_mtime:
        return dest

    try:
        audio = AudioSegment.from_file(src)
        audio.export(dest, format=target_fmt)
    except Exception:
        # Invalid or unsupported file; fall back to simple copy
        import shutil

        shutil.copy2(src, dest)
    os.utime(dest, (src.stat().st_mtime, src.stat().st_mtime))
    return dest


__all__ = ["convert"]


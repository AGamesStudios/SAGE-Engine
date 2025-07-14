"""Audio conversion utility using miniaudio and lameenc."""
from __future__ import annotations

import hashlib
from pathlib import Path

try:
    import miniaudio
    import lameenc
except Exception:  # pragma: no cover - optional deps
    miniaudio = None  # type: ignore
    lameenc = None  # type: ignore


def ogg_to_mp3(path: str, out_dir: str | Path = "build/audio_cache") -> Path:
    """Convert an OGG file to MP3 and return the cached path."""
    if miniaudio is None or lameenc is None:
        raise RuntimeError("miniaudio and lameenc are required for conversion")
    inp = Path(path)
    data = inp.read_bytes()
    h = hashlib.sha1(data + b"mp3").hexdigest()
    out_dir = Path(out_dir)
    out_dir.mkdir(parents=True, exist_ok=True)
    out_file = out_dir / f"{h}.mp3"
    if not out_file.exists():
        decoded = miniaudio.decode(data)
        enc = lameenc.Encoder()
        enc.set_in_sample_rate(decoded.sample_rate)
        enc.set_channels(decoded.nchannels)
        mp3 = enc.encode(decoded.samples.tobytes()) + enc.flush()
        out_file.write_bytes(mp3)
    return out_file

import math
import os
import struct
import wave

from sage_engine.soundmint import convert
from sage_engine.build import sha1


def generate_tone(path: str) -> None:
    framerate = 8000
    duration = 0.05
    nframes = int(duration * framerate)
    with wave.open(path, "wb") as w:
        w.setnchannels(1)
        w.setsampwidth(2)
        w.setframerate(framerate)
        for i in range(nframes):
            val = int(32767 * math.sin(2 * math.pi * 440 * i / framerate))
            w.writeframes(struct.pack("<h", val))


def test_convert_skip(tmp_path):
    src = tmp_path / "tone.wav"
    generate_tone(str(src))
    cwd = os.getcwd()
    os.chdir(tmp_path)
    try:
        dest1 = convert(str(src), "ogg")
        assert dest1.exists()
        expected = tmp_path / "build" / "audio_cache" / f"{sha1(src)}.ogg"
        assert dest1 == expected
        time1 = dest1.stat().st_mtime
        dest2 = convert(str(src), "ogg")
        assert dest2.stat().st_mtime == time1
    finally:
        os.chdir(cwd)


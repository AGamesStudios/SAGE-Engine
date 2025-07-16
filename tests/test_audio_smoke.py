import wave
import math
import struct
import pytest

from sage_engine import audio

pytestmark = pytest.mark.audio


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


def test_play_stop(tmp_path):
    file = tmp_path / "tone.wav"
    generate_tone(str(file))
    handle = audio.play(str(file))
    handle.stop()
    assert not handle.is_playing()

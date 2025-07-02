import os
import wave
import struct
import math

os.environ.setdefault('SDL_AUDIODRIVER', 'dummy')

from engine.audio import AudioManager


def _make_wave(path: str) -> None:
    framerate = 8000
    duration = 0.1
    amp = 32767
    with wave.open(path, 'w') as w:
        w.setnchannels(1)
        w.setsampwidth(2)
        w.setframerate(framerate)
        for i in range(int(duration * framerate)):
            val = int(amp * math.sin(2 * math.pi * 440 * i / framerate))
            w.writeframes(struct.pack('<h', val))


def test_load_and_play(tmp_path):
    wav = tmp_path / 'tone.wav'
    _make_wave(str(wav))
    audio = AudioManager()
    snd = audio.load_sound(str(wav))
    assert snd.get_length() > 0
    audio.play(str(wav))
    audio.stop(str(wav))
    audio.shutdown()

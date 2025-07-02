import os
import wave
import struct
import math
import sys
import types

os.environ.setdefault('SDL_AUDIODRIVER', 'dummy')

if "pygame" not in sys.modules:
    class _DummySound:
        def __init__(self, *a, **k):
            pass

        def play(self):
            pass

        def stop(self):
            pass

        def get_length(self):
            return 1

    class _DummyMixer:
        def __init__(self):
            self._init = False

        def init(self):
            self._init = True

        def quit(self):
            self._init = False

        def get_init(self):
            return self._init

        def Sound(self, path):
            return _DummySound()

    pygame = types.ModuleType("pygame")
    pygame.mixer = _DummyMixer()
    sys.modules["pygame"] = pygame

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


def test_load_descriptor(tmp_path):
    wav = tmp_path / 'tone.wav'
    desc = tmp_path / 'tone.sageaudio'
    _make_wave(str(wav))
    desc.write_text('{"file": "tone.wav"}')
    audio = AudioManager()
    snd = audio.load_sound(str(desc))
    assert snd.get_length() > 0
    audio.shutdown()

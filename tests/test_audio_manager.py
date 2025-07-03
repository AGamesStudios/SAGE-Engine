import os
import wave
import struct
import math
import sys
import types
import pytest

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

        def set_volume(self, vol):
            self.volume = vol

    class _DummyMusic:
        def load(self, *a, **k):
            pass

        def play(self, *a, **k):
            pass

        def stop(self):
            pass

        def pause(self):
            pass

        def unpause(self):
            pass

        def set_volume(self, vol):
            self.volume = vol

    class _DummyMixer:
        def __init__(self):
            self._init = False
            self.music = _DummyMusic()

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


def test_volume_metadata(tmp_path):
    wav = tmp_path / 'tone.wav'
    desc = tmp_path / 'tone.sageaudio'
    _make_wave(str(wav))
    desc.write_text('{"file": "tone.wav", "volume": 0.7}')
    audio = AudioManager()
    snd = audio.load_sound(str(desc))
    vol_attr = getattr(snd, "volume", None)
    if vol_attr is not None:
        assert vol_attr == 0.7
    else:
        assert abs(snd.get_volume() - 0.7) < 0.01
    audio.shutdown()


def test_music_controls(tmp_path):
    wav = tmp_path / 'tone.wav'
    _make_wave(str(wav))
    audio = AudioManager()
    audio.load_music(str(wav))
    audio.play_music(loops=1)
    audio.set_music_volume(0.5)
    audio.pause_music()
    audio.unpause_music()
    audio.stop_music()
    audio.shutdown()


def test_invalid_music_path(tmp_path):
    audio = AudioManager()
    bad = tmp_path / "missing.wav"

    class Boom(Exception):
        pass

    def fail(path):
        raise Boom(path)

    import pygame

    old_load = pygame.mixer.music.load
    pygame.mixer.music.load = fail
    try:
        with pytest.raises(Boom):
            audio.play_music(str(bad))
    finally:
        pygame.mixer.music.load = old_load

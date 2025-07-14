import wave
import struct
import math

from engine import play_sound


def make_wave(path: str) -> None:
    rate = 8000
    duration = 0.1
    with wave.open(path, 'w') as w:
        w.setnchannels(1)
        w.setsampwidth(2)
        w.setframerate(rate)
        for i in range(int(rate * duration)):
            val = int(32767 * math.sin(2 * math.pi * 440 * i / rate))
            w.writeframes(struct.pack('<h', val))


def test_play_and_pitch(tmp_path):
    wav = tmp_path / 'tone.wav'
    make_wave(str(wav))
    snd = play_sound(str(wav), gain=0.5)
    snd.set_pitch(1.2)
    snd.stop()
    assert snd._device is None


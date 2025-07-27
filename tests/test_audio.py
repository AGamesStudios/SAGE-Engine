import os
import wave
import numpy as np
import soundfile as sf
import pytest

from sage_engine import audio

@pytest.mark.audio
def test_audio_play(tmp_path):
    wav_path = tmp_path / "tone.wav"
    ogg_path = tmp_path / "tone.ogg"
    # generate simple sine wave
    sample_rate = 22050
    t = np.linspace(0, 0.1, int(sample_rate * 0.1), False)
    tone = (np.sin(2 * np.pi * 440 * t) * 32767).astype(np.int16)
    with wave.open(wav_path, "wb") as f:
        f.setnchannels(1)
        f.setsampwidth(2)
        f.setframerate(sample_rate)
        f.writeframes(tone.tobytes())
    sf.write(ogg_path, tone.astype(np.float32), sample_rate, format="OGG", subtype="VORBIS")

    audio.set_backend("null")
    audio.audio_boot()
    h1 = audio.play(str(wav_path))
    h2 = audio.play(str(ogg_path), volume=0.5, loop=False)
    assert h1.device.backend.lower() == "null"
    assert h2.device.backend.lower() == "null"
    audio.stop(h1)
    audio.stop(h2)
    audio.audio_shutdown()

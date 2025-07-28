from sage_engine import audio


def test_audio_play(capsys):
    audio.audio.play("beep.wav")
    out = capsys.readouterr().out
    assert "beep.wav" in out

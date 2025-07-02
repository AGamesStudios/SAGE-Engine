import types

import engine.logic.actions.playsound as playsound_mod


def test_play_sound_executes():
    calls = {}

    class DummyAM:
        def __init__(self):
            pass

        def play(self, name):
            calls['name'] = name

    # Patch AudioManager
    playsound_mod.AudioManager = DummyAM
    action = playsound_mod.PlaySound('tone.wav')
    eng = types.SimpleNamespace()
    action.execute(eng, None, 0)
    assert calls['name'] == 'tone.wav'
    assert isinstance(eng._audio_manager, DummyAM)

import types

import sage_engine.logic.actions.playsound as playsound_mod

class DummySound:
    def __init__(self):
        self.stopped = False
    def set_pitch(self, p):
        pass
    def stop(self):
        self.stopped = True



def test_play_sound_executes():
    calls = {}

    def dummy_play(name, **_):
        calls['name'] = name

    playsound_mod.play_sound = dummy_play
    action = playsound_mod.PlaySound('tone.wav')
    eng = types.SimpleNamespace()
    action.execute(eng, None, 0)
    assert calls['name'] == 'tone.wav'

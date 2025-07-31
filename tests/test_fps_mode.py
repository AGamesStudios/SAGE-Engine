import importlib
import sage_engine.runtime.fps as fps


def test_get_current_fps():
    fps.fps_mode = fps.FPSMode.CLASSIC
    assert abs(fps.get_current_fps(0.1) - 10.0) < 0.001
    fps.fps_mode = fps.FPSMode.GAMEFRAME
    assert abs(fps.get_current_fps(0.1, gf_index=80, target_fps=60) - 48.0) < 0.001
    fps.fps_mode = fps.FPSMode.CLASSIC

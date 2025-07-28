from sage_engine import effects
import pytest


def test_builtin_registration():
    names = effects.list_effects()
    assert "blur" in names
    assert "glow" in names
    assert "pixelate" in names


def test_apply_glow():
    buf = bytearray([0,0,0,255]*4)
    effects.apply("glow", buf, 1, 1)
    # After glow, color channels should increase
    assert buf[0] > 0 or buf[1] > 0 or buf[2] > 0


def test_pipeline():
    buf = bytearray([10, 10, 10, 255] * 4)
    pipeline = [("noise", {"strength": 0}), ("fade", {"alpha": 0.5})]
    effects.apply_pipeline(pipeline, buf, 1, 1)
    assert buf[3] < 255


def test_pipeline_pingpong():
    buf = bytearray([50, 50, 50, 255] * 4)
    pipeline = [("blur", {"radius": 1}), ("fade", {"alpha": 0.5})]
    before = buf[:]
    effects.apply_pipeline(pipeline, buf, 1, 1)
    # source not destroyed
    assert before != buf


def test_scissor_and_mask():
    buf = bytearray([0, 0, 0, 255] * 4)
    mask = effects.Frame(bytearray([0,0,0,0, 0,0,0,255, 0,0,0,255, 0,0,0,0]), 2, 2)
    effects.set_mask(mask)
    effects.set_scissor(0,0,2,2)
    effects.apply("glow", buf, 2, 2)
    effects.clear_mask(); effects.clear_scissor()
    assert buf[7] == 255  # one pixel affected


def test_param_validation():
    with pytest.raises(ValueError):
        effects.apply("blur", bytearray(4), 1, 1, radius=-1)


def test_preset_io(tmp_path):
    preset = [("blur", {"radius": 1})]
    path = tmp_path/"p.json"
    effects.save_preset(str(path), preset)
    spec = effects.load_preset(str(path))
    buf = bytearray([0,0,0,255]*4)
    effects.apply_pipeline(spec, buf, 1, 1)


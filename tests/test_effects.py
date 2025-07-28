from sage_engine import effects


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


def test_smart_fxaa():
    from sage_engine.graphic import smoothing

    buf = bytearray([255, 0, 0, 255] + [0, 0, 0, 255] * 3)
    smoothing.smart_fxaa(buf, 2, 2)
    # smoothing should average colors so first pixel less than 255 red
    assert buf[2] < 255

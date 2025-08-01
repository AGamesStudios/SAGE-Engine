def test_fill_gradient():
    from sage_engine.graphic import gradient
    from sage_engine import gfx

    gfx.init(2, 2)
    gfx.begin_frame()
    gradient.fill_rect_gradient(0, 0, 2, 2, (0, 0, 0, 255), (255, 0, 0, 255))
    buf = gfx.end_frame()
    assert buf[2] == 0      # first line black
    assert buf[2 + 2*4] > 0  # bottom line red
    gfx.shutdown()

from sage_engine.render.backends.software import SoftwareRenderer


def test_draw_rect():
    r = SoftwareRenderer(10, 10)
    r.begin_frame()
    r.draw_rect(2, 3, 4, 2, (1, 2, 3))
    block = r.end_frame()
    assert block.pixels[3][2] == (1, 2, 3)
    assert block.pixels[4][5] == (1, 2, 3)
    # Outside the rectangle should remain unchanged
    assert block.pixels[3][1] == (0, 0, 0)


def test_draw_text():
    r = SoftwareRenderer(20, 20)
    r.begin_frame()
    r.draw_text(0, 0, "A", (9, 9, 9))
    block = r.end_frame()
    # draw_text renders a 6x8 block starting at the provided coordinates
    assert block.pixels[0][0] == (9, 9, 9)
    assert block.pixels[7][5] == (9, 9, 9)
    # A pixel just outside the drawn area should remain blank
    assert block.pixels[0][7] == (0, 0, 0)

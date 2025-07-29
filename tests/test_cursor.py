from sage_engine.cursor import CursorRenderer


def test_position_follow():
    c = CursorRenderer()
    c.state.follow_rate = 1.0
    c.state.set_position(10, 20)
    assert c.state.get_position() == (10, 20)
    c.state.follow_rate = 0.5
    c.state.set_position(14, 28)
    assert c.state.get_position() == (12, 24)


def test_visibility_and_style():
    c = CursorRenderer()
    c.state.set_visible(False)
    assert not c.state.visible
    c.state.set_style("retro")
    assert c.state.style == "retro"

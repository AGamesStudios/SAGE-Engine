from sage_engine.cursor import set_style, CursorRenderer, _renderer


def test_set_style_runtime():
    r = CursorRenderer()
    set_style("cross")
    assert r.state.style == "default"
    assert _renderer.state.style == "cross"

import sage_engine.render as render


def test_set_clear_color():
    render.boot()
    render.set_clear_color(10, 20, 30, 255)
    render.render_scene([])
    assert render._clear_color == (10, 20, 30, 255)

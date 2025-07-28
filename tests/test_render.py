from sage_engine import render
from sage_engine.settings import settings


def test_basic_draw_calls():
    settings.render_backend = "software"
    render.init(None)
    render.begin_frame()
    render.draw_rect(0, 0, 10, 10, (1, 0, 0, 1))
    render.draw_sprite("img", 5, 5, 8, 8)
    render.end_frame()
    backend = render._get_backend()
    assert backend.commands[0] == "begin"
    assert backend.commands[-1] == "end"
    assert ("rect", 0, 0, 10, 10, (1, 0, 0, 1)) in backend.commands
    render.shutdown()

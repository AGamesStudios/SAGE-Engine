import sage_engine.render as render
import sage_engine.sprites as sprites
from sage_engine import ui


def test_ui_overlay():
    backend = render.get_backend("headless")
    backend.create_device(16, 16)
    sprites.clear()
    ui.theme.set_theme(str(ui.theme._current_path))
    sprites.add(sprites.Sprite(x=0.0, y=0.0))
    ui.Button()
    render.draw_frame(backend)
    assert backend.draw_calls == 2
    assert backend.frames == 1

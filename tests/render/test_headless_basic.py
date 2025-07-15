import sage_engine.render as render
import sage_engine.sprites as sprites


def test_headless_basic():
    backend = render.load_backend("headless")
    backend.create_device(16, 16)
    sprites.clear()
    sprites.add(sprites.Sprite(x=1.0, y=2.0))
    # avoid numpy dependency in test
    instances = [[1.0, -0.0, 1.0, 0.0, 1.0, 2.0, 0.0, 1.0, 1.0, 1.0, 1.0]]
    backend.begin_frame()
    backend.draw_sprites(instances)
    backend.end_frame()
    assert backend.draw_calls == 1
    assert backend.frames == 1

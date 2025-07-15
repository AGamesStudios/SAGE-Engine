import pytest

import sage_engine.render as render
import sage_engine.sprites as sprites


def test_opengl_basic():
    pytest.importorskip("moderngl")
    backend = render.load_backend("opengl")
    backend.create_device(32, 32)
    sprites.clear()
    for i in range(10):
        sprites.add(sprites.Sprite(x=float(i), y=0.0))
    instances = sprites.collect_instances()
    backend.begin_frame()
    backend.draw_sprites(instances)
    backend.end_frame()
    assert backend.draw_calls == 1

from pathlib import Path
from sage_engine import core, gfx, render, objects
from sage_engine.texture.texture import Texture
from sage_engine.sprite.sprite import Sprite
from sage_engine.render import stats as render_stats

tex = Texture(1, 1, bytearray([255, 255, 255, 255]))
placeholder = Sprite(tex, (0, 0, 1, 1))


def boot(_cfg):
    render.init()
    objects.runtime.store.objects.clear()
    objects.runtime.store.by_role.clear()
    objects.runtime.store.by_world.clear()
    objects.spawn("Sprite")


def draw():
    gfx.begin_frame()
    gfx.draw_sprite(placeholder, 0, 0)
    render_stats.stats["sprites_drawn"] += 1
    gfx.end_frame()
    gfx.flush_frame()


def test_world_draws_sprite():
    core.core_shutdown()
    core.register("boot", boot)
    core.register("draw", draw)
    core.core_boot()
    core.core_tick()
    stats = core.get("render").stats
    assert stats["sprites_drawn"] > 0
    core.core_shutdown()
    objects.runtime.store.objects.clear()
    objects.runtime.store.by_role.clear()
    objects.runtime.store.by_world.clear()

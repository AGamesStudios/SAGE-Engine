"""Entry point for the SAGE Runner example.

This example purposely uses only placeholder resources.  The game loop is
implemented manually to avoid window freeze and to keep the example explicit.
"""

from pathlib import Path

from sage_engine import core, world, gfx, gui, window, render, objects, transform
from sage_engine.logger import logger
from sage_engine.texture.texture import Texture
from sage_engine.sprite.sprite import Sprite
from sage_engine.transform import runtime as transform_runtime

ROOT = Path(__file__).resolve().parent


_debug_sprite = Sprite(Texture(1, 1, bytearray([255, 0, 0, 255])), (0, 0, 1, 1))


def boot(cfg):
    """Load the example world and enable GUI debugging."""
    logger.debug("phase boot")
    window.init(title="SAGE Runner", width=640, height=360)
    render.init(window.get_window_handle())
    w, h = window.get_framebuffer_size()
    gfx.init(w, h)
    cam = transform.Camera2D(pos=(0.0, 0.0), viewport_px=(w, h), zoom=1.0)
    render.set_camera(cam)
    print("Загружаем мир level1.sageworld...")
    data = world.load(ROOT / "world" / "level1.sageworld")
    for entry in data:
        obj = objects.spawn("Sprite")
        t = entry.get("transform", {})
        obj.position.x = t.get("x", 0)
        obj.position.y = t.get("y", 0)
    objs = list(objects.runtime.store.objects.values())
    transform_runtime.register_all(objs)
    print("World loaded.")
    print("Objects in world:", len(objs))
    for obj in objs:
        print("-", obj.name or obj.id, "at", obj.position.x, obj.position.y)
    gui.manager.debug = True
    print("GUI overlay enabled.")
    print("Boot completed.")


def draw():
    logger.debug("phase draw")
    gfx.begin_frame()
    render.begin_frame()
    for obj in objects.runtime.store.objects.values():
        logger.debug(f"Transforming {obj.id}")
        gfx.draw_sprite(_debug_sprite, int(obj.position.x), int(obj.position.y))
        logger.debug(f"Submitted to render: {obj.id}")
    gfx.draw_rect(0, 0, 8, 8, (0, 255, 0, 255))
    gfx.end_frame()
    gfx.flush_frame()
    render.end_frame()
    stats = core.get("render").stats
    print("render stats:", stats)


core.register("boot", boot)
core.register("draw", draw)
core.register("update", lambda: logger.debug("phase update"))
core.register("flush", lambda: logger.debug("phase flush"))
core.register("shutdown", lambda: logger.debug("phase shutdown"))


if __name__ == "__main__":  # pragma: no cover - manual example run
    core.core_boot()
    try:
        while not window.should_close():
            window.poll_events()
            core.core_tick()
    finally:
        core.core_shutdown()

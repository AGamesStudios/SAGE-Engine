from __future__ import annotations
import random
from pathlib import Path
from sage_engine import window, render, gfx, objects
from sage_engine.roles import load_json_roles
from sage_engine.runtime import FrameSync
from sage_engine.resource.loader import load_cfg
from sage_engine.logger import logger


def load_config(path: str | None = None):
    if path is None:
        path = Path(__file__).with_name("game.sagecfg")
    cfg = load_cfg(path)
    name = cfg.get("name", "Render Test")
    width = int(cfg.get("width", 640))
    height = int(cfg.get("height", 360))
    count = int(cfg.get("count", 10000))
    return name, width, height, count


class RectObject(objects.roles.interfaces.Role):
    """Simple rectangular role used for the stress test."""

    def __init__(self,
        *,
        x: int = 0,
        y: int = 0,
        width: int = 10,
        height: int = 10,
        color: tuple[int, int, int, int] | None = None,
    ) -> None:
        self.start_x = x
        self.start_y = y
        self.width = width
        self.height = height
        self.color = color or (255, 255, 255, 255)
        self._obj: objects.object.Object | None = None

    def on_attach(self, obj: objects.object.Object) -> None:  # type: ignore[override]
        self._obj = obj
        obj.position.x = float(self.start_x)
        obj.position.y = float(self.start_y)

    def on_render(self, ctx) -> None:  # type: ignore[override]
        if self._obj:
            x = int(self._obj.position.x)
            y = int(self._obj.position.y)
            gfx.draw_rect(x, y, self.width, self.height, self.color)


def main() -> None:
    name, width, height, count = load_config()
    load_json_roles(Path(__file__).parent / "roles")
    objects.roles.register("RectObject", RectObject)

    window.init(name, width, height)
    render.init(window.get_window_handle())
    gfx.init(width, height)

    for _ in range(count):
        x = random.randint(0, width - 10)
        y = random.randint(0, height - 10)
        color = [random.randint(0, 255) for _ in range(3)] + [255]
        objects.spawn("RectObject", x=x, y=y, width=10, height=10, color=color)
    logger.info("[INFO] Spawned %d test rectangles", count)

    fsync = FrameSync(target_fps=60)
    frames = 0
    total = 0.0
    running = True
    while running:
        window.poll_events()
        fsync.start_frame()
        gfx.begin_frame((0, 0, 0, 255))

        # iterate over object instances and render each
        for obj in objects.runtime.store.objects.values():
            obj.render(None)

        gfx.flush_frame(window.get_window_handle())
        fsync.end_frame()
        frames += 1
        total += fsync.target_dt
        if window.should_close():
            running = False

    if frames:
        logger.info("[INFO] Average frame time: %.2f ms", (total / frames) * 1000)

    gfx.shutdown()
    render.shutdown()
    window.shutdown()


if __name__ == "__main__":
    main()

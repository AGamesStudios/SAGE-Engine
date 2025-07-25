import os
import sys
import yaml

from sage import emit

sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), "../..")))

from sage.config import DEFAULT_WINDOW_CONFIG
from sage_engine import core_boot, core_reset, framesync, time, input
from sage_engine.render import render_scene
from sage_engine.object import get_objects
from sage_engine.window import poll as poll_window, present as present_window, should_close, get_window


def load_cfg() -> None:
    path = os.path.join(os.path.dirname(__file__), "config.yaml")
    if os.path.isfile(path):
        data = yaml.safe_load(open(path, encoding="utf-8")) or {}
        DEFAULT_WINDOW_CONFIG.update(data.get("window", {}))


def main() -> None:
    load_cfg()
    core_boot()
    time.mark()
    try:
        while not should_close():
            poll_window()
            if input.input_key_down("escape"):
                get_window().close()
            dt = time.get_delta()
            input.update()
            emit("update", dt)
            render_scene(get_objects())
            present_window()
            framesync.regulate()
            time.mark()
    finally:
        core_reset()


if __name__ == "__main__":
    main()

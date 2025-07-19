import os
import sys
import yaml

# allow running directly from this folder
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), "../..")))

from sage.config import DEFAULT_WINDOW_CONFIG
from sage import emit
from sage_engine import core_boot, core_reset, framesync, input, time
from sage_engine.render import render_scene
from sage_engine.object import get_objects
from sage_engine.window import get_window


def load_cfg():
    path = os.path.join(os.path.dirname(__file__), "config.yaml")
    if os.path.isfile(path):
        data = yaml.safe_load(open(path, encoding="utf-8")) or {}
        DEFAULT_WINDOW_CONFIG.update(data.get("window", {}))


if os.name == "nt":
    import msvcrt

    def poll_key() -> str | None:
        if msvcrt.kbhit():
            ch = msvcrt.getch().decode("utf-8")
            return ch
        return None
else:
    import sys as _sys
    import select
    import termios
    import tty

    def poll_key() -> str | None:
        dr, _, _ = select.select([_sys.stdin], [], [], 0)
        if dr:
            fd = _sys.stdin.fileno()
            old = termios.tcgetattr(fd)
            try:
                tty.setraw(fd)
                ch = _sys.stdin.read(1)
            finally:
                termios.tcsetattr(fd, termios.TCSADRAIN, old)
            return ch
        return None


def main() -> None:
    load_cfg()
    core_boot()
    emit("ready")
    time.mark()

    try:
        while not get_window().should_close:
            key = poll_key()
            if key:
                up_key = key.upper()
                if up_key == "\x1b":
                    get_window().close()
                else:
                    input.press_key(up_key)
            dt = time.get_delta()
            emit("update", dt)
            print("draw calls:", render_scene(get_objects()))
            framesync.regulate()
            time.mark()
            if key:
                input.release_key(key.upper())
    finally:
        core_reset()


if __name__ == "__main__":
    main()

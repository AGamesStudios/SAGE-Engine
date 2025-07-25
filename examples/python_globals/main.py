from sage import emit
from sage_engine import core_boot, core_reset, framesync, time, set_python_globals
from sage_engine.render import render_scene
from sage_engine.object import get_objects
from sage_engine.window import poll, present


def main() -> None:
    set_python_globals(log=print)
    core_boot()
    emit("ready")
    time.mark()
    try:
        for _ in range(1):
            poll()
            dt = time.get_delta()
            emit("update", dt)
            render_scene(get_objects())
            present()
            framesync.regulate()
            time.mark()
    finally:
        core_reset()


if __name__ == "__main__":
    main()

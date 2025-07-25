from sage import emit
from sage_engine import core_boot, core_reset, framesync, time, input
from sage_engine.render import render_scene
from sage_engine.object import get_objects
from sage_engine.window import poll, present, should_close, get_window


def main() -> None:
    core_boot()
    time.mark()
    try:
        while not should_close():
            poll()
            if input.is_key_down("escape"):
                get_window().close()
            dt = time.get_delta()
            input.input_poll()
            emit("update", dt)
            render_scene(get_objects())
            present()
            framesync.regulate()
            time.mark()
    finally:
        core_reset()


if __name__ == "__main__":
    main()

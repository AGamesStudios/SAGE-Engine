"""Pixel Signals demo game entry point."""

from sage_engine import window, render, graphic, sprite, resource, runtime
from sage_engine.animation import AnimationPlayer
from sage_engine.graphic import state as gfx_state


def load_resources():
    resource.load("demo.assets")  # placeholder for resource loading


def main():
    window.init("Pixel Signals", 320, 240)
    render.init(window.get_window_handle())
    gfx_state.set_state("style", "neo-retro")

    load_resources()

    running = True
    while running:
        window.poll_events()
        runtime.fsync.start_frame()
        graphic.api.draw_sprite(None, 0, 0)  # placeholder
        graphic.api.flush()
        runtime.fsync.end_frame()

        if window.should_close():
            running = False

    render.shutdown()
    window.shutdown()


if __name__ == "__main__":
    main()

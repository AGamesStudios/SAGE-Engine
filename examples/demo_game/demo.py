"""Demo game showcasing SAGE Render and Window usage."""

import asyncio
import os
from sage_engine.logger import logger

from sage_engine import (
    core,
    world,
    render,
    events,
    scheduler,
    tasks,
    audio,
    window,
)
from sage_engine.flow.python import run as run_flow

# register boot/update phases
core.register("boot", scheduler.time.boot)
core.register("boot", scheduler.timers.boot)
core.register("boot", events.boot)
core.register("boot", world.boot)
core.register("boot", tasks.boot)

core.register("update", scheduler.time.update)
core.register("update", scheduler.timers.update)
core.register("update", events.update)
core.register("update", tasks.update)
core.register("update", world.update)



def render_frame() -> None:
    """Render one frame using the active render backend."""
    # begin_frame/end_frame ensure proper batching and swap buffers
    render.begin_frame()
    render.draw_rect(50, 50, 100, 100, (255, 0, 0))
    render.end_frame()


# register our render function to be called each frame
core.register("draw", render_frame)


def main() -> None:
    """Run a minimal game loop with window events."""
    # create the main window and initialize rendering with its handle
    window.init("Demo", 640, 480)
    render.init(window.get_window_handle())
    core.core_boot({})

    edit = world.scene.begin_edit()
    # create a sprite object via SceneEdit
    edit.create(role="sprite", name="hero", tex_id=1, x=0, y=0)
    world.scene.apply(edit)

    # play a sound if the file exists
    if os.path.exists("start.wav"):
        logger.info("Playing start.wav")
        audio.audio.play("start.wav")
    else:
        logger.warn("start.wav not found")

    # flag controlled by window:close or ESC
    done = False

    def mark_done() -> None:
        nonlocal done
        done = True
        logger.info("Window closed")

    # subscribe to window events via the global dispatcher
    events.dispatcher.on(window.WIN_RESIZE, lambda w, h: logger.info("Resized to %dx%d", w, h))

    def on_key(key: str, code: int) -> None:
        """Handle keyboard input."""
        logger.debug("Key down: %s", key)
        if key == "Escape":
            # allow closing the game with ESC
            mark_done()

    events.dispatcher.on(window.WIN_KEY, on_key)
    events.dispatcher.on(
        window.WIN_MOUSE,
        lambda t, x, y, b: logger.debug("Mouse %s %d %d button=%d", t, x, y, b),
    )

    # closing the window or pressing ESC will end the loop
    events.dispatcher.on(window.WIN_CLOSE, mark_done)

    # FlowScript demo: run a simple script once
    asyncio.run(run_flow("ctx['done'] = True", {"ctx": {"done": False}}))

    # the loop ends when should_close() is True or window:close sets done
    logger.info("Starting main loop")
    while not done and not window.should_close():
        window.poll_events()
        core.core_tick()
    logger.info("Exiting main loop")

    # close the window, render and modules
    render.shutdown()
    window.shutdown()
    core.core_shutdown()


if __name__ == "__main__":
    main()

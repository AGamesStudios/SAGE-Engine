"""Demo game showcasing SAGE Render and Window usage."""

import asyncio
import os
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
    # create the main window
    window.init("Demo", 640, 480)
    core.core_boot({})

    edit = world.scene.begin_edit()
    # create a sprite object via SceneEdit
    edit.create(role="sprite", name="hero", tex_id=1, x=0, y=0)
    world.scene.apply(edit)

    # play a sound if the file exists
    if os.path.exists("start.wav"):
        audio.audio.play("start.wav")
    else:
        print("[WARN] start.wav not found")

    # flag controlled by window:close or ESC
    done = False

    def mark_done() -> None:
        nonlocal done
        done = True
        print("Window closed")

    # subscribe to window events via the global dispatcher
    events.dispatcher.on(window.WIN_RESIZE, lambda w, h: print(f"Resized to {w}x{h}"))

    def on_key(key: str, code: int) -> None:
        """Handle keyboard input."""
        print(f"Key down: {key}")
        if key == "Escape":
            # allow closing the game with ESC
            mark_done()

    events.dispatcher.on(window.WIN_KEY, on_key)
    events.dispatcher.on(
        window.WIN_MOUSE,
        lambda t, x, y, b: print(f"Mouse {t} {x} {y} button={b}"),
    )

    # closing the window or pressing ESC will end the loop
    events.dispatcher.on(window.WIN_CLOSE, mark_done)

    # FlowScript demo: run a simple script once
    asyncio.run(run_flow("ctx['done'] = True", {"ctx": {"done": False}}))

    # the loop ends when should_close() is True or window:close sets done
    while not done and not window.should_close():
        # process OS events every frame so the window stays responsive
        window.poll_events()
        # core_tick runs update -> draw -> flush phases
        core.core_tick()

    # close the window and clean up modules
    window.shutdown()
    core.core_shutdown()


if __name__ == "__main__":
    main()

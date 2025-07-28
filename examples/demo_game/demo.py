"""Demo game showcasing SAGE Window usage."""

import asyncio
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

core.register("draw", lambda: render.prepare(world.scene))
core.register("draw", render.sort)
core.register("flush", render.flush)


def main() -> None:
    """Run a minimal game loop with window events."""
    # create the main window
    window.init("Demo", 640, 480)
    core.core_boot({})

    edit = world.scene.begin_edit()
    # create a sprite object via SceneEdit
    edit.create(role="sprite", name="hero", tex_id=1, x=0, y=0)
    world.scene.apply(edit)
    audio.audio.play("start.wav")

    # log events from the window subsystem
    events.dispatcher.on(window.WIN_RESIZE, lambda w, h: print(f"Resized to {w}x{h}"))
    events.dispatcher.on(window.WIN_KEY, lambda key, code: print(f"Key down: {key}"))
    events.dispatcher.on(
        window.WIN_MOUSE,
        lambda t, x, y, b: print(f"Mouse {t} {x} {y} button={b}"),
    )

    # flag controlled by window:close event
    done = False

    def mark_done() -> None:
        nonlocal done
        done = True
        print("Window closed")

    events.dispatcher.on(window.WIN_CLOSE, mark_done)

    # FlowScript demo: run a simple script once
    asyncio.run(run_flow("ctx['done'] = True", {"ctx": {"done": False}}))

    # the loop ends when should_close() is True or window:close sets done
    while not done and not window.should_close():
        # poll_events keeps the window responsive without heavy CPU load
        window.poll_events()
        core.core_tick()  # update, draw and flush

    # close the window and clean up modules
    window.shutdown()
    core.core_shutdown()


if __name__ == "__main__":
    main()

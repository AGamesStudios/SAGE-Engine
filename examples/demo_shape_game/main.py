from sage_engine import window, render, gfx
from sage_engine.runtime import FrameSync
from sage_engine.input import Input
import logic


def main() -> None:
    window.init("Shape Shooter", logic.WIDTH, logic.HEIGHT)
    render.init(window.get_window_handle())
    gfx.init(logic.WIDTH, logic.HEIGHT)

    Input.map_action("left", "LEFT")
    Input.map_action("right", "RIGHT")
    Input.map_action("shoot", "SPACE")

    fsync = FrameSync(target_fps=60)
    logic.init_game()

    running = True
    while running:
        window.poll_events()
        Input.poll()

        fsync.start_frame()
        gfx.begin_frame((30, 30, 30, 255))

        running = running and logic.update(1 / 60.0, Input)
        logic.draw()

        gfx.flush_frame(window.get_window_handle(), fsync)

        if window.should_close():
            running = False

    render.shutdown()
    window.shutdown()


if __name__ == "__main__":
    main()

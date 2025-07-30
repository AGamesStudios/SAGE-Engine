from sage_engine import window, render, gfx
from sage_engine.runtime import FrameSync
from sage_engine.input import Input
from sage_engine.flow.runtime import run_flow_script

WIDTH = 640
HEIGHT = 360


def main() -> None:
    window.init("FlowScript Demo", WIDTH, HEIGHT)
    render.init(window.get_window_handle())
    gfx.init(WIDTH, HEIGHT)
    Input.init(window.get_window_handle())

    Input.map_action("left", "LEFT")
    Input.map_action("right", "RIGHT")

    fsync = FrameSync(target_fps=60)
    run_flow_script("logic.flow", Input, lang="ru")

    running = True
    while running:
        window.poll_events()
        Input.poll()

        fsync.start_frame()
        gfx.begin_frame((0, 0, 0, 255))

        state = run_flow_script("logic.flow", Input, lang="ru")
        x = int(state.get("player_x", WIDTH // 2))
        gfx.draw_rect(x, HEIGHT - 40, 20, 10, (0, 255, 255, 255))

        gfx.flush_frame(window.get_window_handle(), fsync)
        if window.should_close():
            running = False

    render.shutdown()
    window.shutdown()


if __name__ == "__main__":
    main()

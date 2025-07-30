from sage_engine import window, render, gfx
from sage_engine.runtime import FrameSync
from sage_engine.input import Input
from sage_engine.flow.runtime import run_flow_script
from sage_engine.resource.loader import load_cfg


def main() -> None:
    cfg = load_cfg("game.sagecfg")
    name = cfg.get("название", "SAGE Game")
    width = int(cfg.get("ширина", 640))
    height = int(cfg.get("высота", 360))
    script = cfg.get("скрипт", "logic.flow")

    window.init(name, width, height)
    render.init(window.get_window_handle())
    gfx.init(width, height)
    Input.init(window.get_window_handle())

    Input.map_action("left", "LEFT")
    Input.map_action("right", "RIGHT")
    Input.map_action("shoot", "SPACE")

    fsync = FrameSync(target_fps=60)
    run_flow_script(script, Input, lang="ru")

    running = True
    while running:
        window.poll_events()
        Input.poll()

        fsync.start_frame()
        gfx.begin_frame((0, 0, 0, 255))

        state = run_flow_script(script, Input, lang="ru")
        x = int(state.get("player_x", width // 2))
        gfx.draw_rect(x, height - 40, 20, 10, (0, 255, 255, 255))

        gfx.flush_frame(window.get_window_handle(), fsync)
        if window.should_close():
            running = False

    render.shutdown()
    window.shutdown()


if __name__ == "__main__":
    main()

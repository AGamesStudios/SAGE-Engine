from sage_engine import window, render, gfx
from sage_engine.runtime import FrameSync
from sage_engine.input import Input
from sage_engine.flow.runtime import run_flow_script
from pathlib import Path
from sage_engine.resource.loader import load_cfg
from sage_engine.logger import logger


def load_config(path: str | None = None):
    if path is None:
        path = Path(__file__).with_name("game.sagecfg")
    data = load_cfg(path)
    allowed = {"игра", "скрипт", "язык", "разрешение"}
    for key in list(data.keys()):
        if key not in allowed:
            logger.error("[config] Неизвестное поле в конфиге: \"%s\"", key)
    name = data.get("игра", "FlowScript Demo")
    script = data.get("скрипт", "logic.flow")
    lang = data.get("язык", "ru")
    res = data.get("разрешение", [640, 360])
    width = int(res[0])
    height = int(res[1]) if len(res) > 1 else width
    return name, script, lang, width, height


def main() -> None:
    name, script, lang, width, height = load_config()

    window.init(name, width, height)
    render.init(window.get_window_handle())
    gfx.init(width, height)
    Input.init(window.get_window_handle())

    fsync = FrameSync(target_fps=60)
    run_flow_script(script, Input, lang=lang)

    running = True
    while running:
        window.poll_events()
        Input.poll()

        fsync.start_frame()
        gfx.begin_frame((0, 0, 0, 255))

        state = run_flow_script(script, Input, lang=lang)
        x = int(state.get("player_x", width // 2))
        gfx.draw_rect(x, height - 40, 20, 10, (0, 255, 255, 255))

        gfx.flush_frame(window.get_window_handle(), fsync)
        if window.should_close():
            running = False

    render.shutdown()
    window.shutdown()


if __name__ == "__main__":
    main()

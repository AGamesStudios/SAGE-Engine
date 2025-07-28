from sage_engine import window, render


def main() -> None:
    window.init("Render Test", 400, 300)
    render.init(window.get_window_handle())
    pos = 0
    while not window.should_close() and pos < 200:
        window.poll_events()
        render.begin_frame()
        render.draw_rect(pos, 20, 40, 40, (0, 255, 0))
        render.end_frame()
        pos += 5
    render.shutdown()
    window.shutdown()


if __name__ == "__main__":
    main()

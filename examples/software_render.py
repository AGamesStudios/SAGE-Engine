from sage_engine import window, render, gfx


def main():
    window.init("SAGE Graphic Test", 640, 480)
    gfx.init(640, 480)
    render.init(window.get_window_handle())
    x = 0
    while not window.should_close():
        window.poll_events()
        gfx.begin_frame()
        gfx.draw_rect(x, 100, 50, 50, (0, 255, 0))
        buffer = gfx.end_frame()
        render.present(buffer)
        x = (x + 5) % 640
    gfx.shutdown()
    render.shutdown()
    window.shutdown()


if __name__ == "__main__":
    main()

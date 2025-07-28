from sage_engine import window, gfx


def main():
    window.init("SAGE Graphic Test", 640, 480)
    gfx.init(window.get_window_handle())
    x = 0
    while not window.should_close():
        window.poll_events()
        gfx.begin_frame()
        gfx.draw_rect(x, 100, 50, 50, (0, 255, 0))
        gfx.end_frame()
        x = (x + 5) % 640
    gfx.shutdown()
    window.shutdown()


if __name__ == "__main__":
    main()

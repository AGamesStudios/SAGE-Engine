from sage_engine import window, render, gfx
from sage_engine.runtime import FrameSync
from sage_engine.input import Input
from sage_engine.world.runtime import WorldRuntime

WIDTH = 640
HEIGHT = 360


def main() -> None:
    window.init("World Demo", WIDTH, HEIGHT)
    render.init(window.get_window_handle())
    gfx.init(WIDTH, HEIGHT)
    Input.init(window.get_window_handle())

    Input.map_action("left", "LEFT")
    Input.map_action("right", "RIGHT")

    world = WorldRuntime()
    world.load("level1.sagecfg")

    fsync = FrameSync(target_fps=60)

    running = True
    while running:
        window.poll_events()
        Input.poll()

        fsync.start_frame()
        gfx.begin_frame((0, 0, 0, 255))

        # draw objects from world runtime
        for obj in world.objects:
            if obj.roles and "shape" in obj.roles:
                gfx.draw_rect(int(obj.x), int(obj.y), 20, 10, (0, 255, 255, 255))

        gfx.flush_frame(window.get_window_handle(), fsync)

        if window.should_close():
            running = False

    render.shutdown()
    window.shutdown()


if __name__ == "__main__":
    main()

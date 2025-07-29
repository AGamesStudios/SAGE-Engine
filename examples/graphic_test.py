from sage_engine import window, render, gfx
from sage_engine.graphic.scene import Scene, Layer
from sage_engine.graphic import fx


def main():
    window.init("Graphic Test", 320, 240)
    gfx.init(320, 240)
    render.init(window.get_window_handle())

    scene = Scene()
    base = Layer(z=0)
    top = Layer(z=10)
    scene.add(base)
    scene.add(top)

    scene.rect(base, 30, 30, 100, 60, (0, 0, 255, 200))
    scene.rect(top, 50, 50, 100, 60, "#FF000080")

    x = 0
    while not window.should_close() and x < 120:
        window.poll_events()
        gfx.begin_frame((0, 0, 0, 255))
        scene.render()
        scene.rect(top, 80 + x, 80, 40, 40, (0, 255, 0, 128))
        gfx.draw_circle(160, 120, 20, (255, 255, 0, 180))
        gfx.draw_line(0, 0, 319, 239, (255, 0, 255, 255))
        buffer = gfx.end_frame()
        render.present(buffer)
        x += 2

    gfx.shutdown()
    render.shutdown()
    window.shutdown()

if __name__ == "__main__":
    main()

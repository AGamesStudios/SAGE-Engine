from sage_engine import window, render, gfx
from sage_engine.graphic.scene import Scene, Layer

window.init("Graphic Demo", 320, 240)
gfx.init(320, 240)
render.init(window.get_window_handle())

scene = Scene()
background = Layer(z=0)
mid = Layer(z=5)
front = Layer(z=10)
scene.add(background)
scene.add(mid)
scene.add(front)

scene.rect(background, 0, 0, 320, 240, "#202020")
scene.rect(mid, 50, 50, 60, 40, (255, 0, 0, 128))
scene.rect(mid, 120, 70, 80, 60, (0, 255, 0, 200))

circle_group = scene.group(front)
scene.rect(circle_group, 140, 90, 40, 40, (0, 0, 255, 255))

while not window.should_close():
    window.poll_events()
    gfx.begin_frame()
    scene.render()
    buf = gfx.end_frame()
    render.present(buf)

gfx.shutdown()
render.shutdown()
window.shutdown()

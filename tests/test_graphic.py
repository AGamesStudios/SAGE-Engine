from sage_engine.graphic.color import to_rgba
from sage_engine.gfx.runtime import GraphicRuntime
from sage_engine.graphic import fx
from sage_engine.graphic.scene import Scene, Layer


def test_color_parsing():
    assert to_rgba((1, 2, 3)) == (1, 2, 3, 255)
    assert to_rgba('#01020304') == (1, 2, 3, 4)


def test_alpha_blend():
    rt = GraphicRuntime()
    rt.init(1, 1)
    rt.begin_frame()
    rt.draw_rect(0, 0, 1, 1, (255, 0, 0, 255))
    rt.draw_rect(0, 0, 1, 1, (0, 0, 255, 128))
    # expect some blue mixed with red
    rt.end_frame()
    assert rt.buffer[0] > 0
    assert rt.buffer[2] < 255


def test_z_order():
    rt = GraphicRuntime()
    rt.init(1, 1)
    rt.begin_frame()
    rt.state.z = 0
    rt.draw_rect(0, 0, 1, 1, (0, 0, 255, 255))
    rt.state.z = 1
    rt.draw_rect(0, 0, 1, 1, (255, 0, 0, 255))
    rt.end_frame()
    # red should be on top
    assert rt.buffer[2] == 255  # R
    assert rt.buffer[0] == 0    # B


def test_circle_line():
    rt = GraphicRuntime()
    rt.init(3, 3)
    rt.begin_frame()
    rt.draw_circle(1, 1, 1, (255, 0, 0, 255))
    rt.draw_line(0, 0, 2, 2, (0, 255, 0, 255))
    rt.end_frame()
    # center pixel covered by the line (green)
    off = 1 * rt.pitch + 1 * 4
    assert rt.buffer[off + 1] == 255  # G
    assert rt.buffer[off + 2] == 0    # R overwritten


def test_polygon_and_rounded():
    rt = GraphicRuntime()
    rt.init(5, 5)
    rt.begin_frame()
    rt.draw_polygon([(0,0),(4,0),(2,4)], (255,0,0,255))
    rt.draw_rounded_rect(1,1,3,3,2,(0,255,0,255))
    rt.end_frame()
    off = 2 * rt.pitch + 2 * 4
    # center should be green from rounded rect
    assert rt.buffer[off+1] > 0


def test_state_stack():
    rt = GraphicRuntime()
    rt.init(1,1)
    rt.begin_frame()
    rt.push_state()
    rt.state.color = (255,0,0,255)
    rt.draw_rect(0,0,1,1)
    rt.pop_state()
    rt.state.color = (0,255,0,255)
    rt.draw_rect(0,0,1,1,z=1)
    rt.end_frame()
    # top pixel should be green
    assert rt.buffer[1] == 255


def test_scene_group_z():
    scene = Scene()
    base = Layer(z=0)
    top = Layer(z=1)
    scene.add(base)
    scene.add(top)
    for i in range(100):
        scene.rect(base, 0, 0, 1, 1, (0,0,255,255))
    g = scene.group(top)
    scene.rect(g, 0, 0, 1, 1, (255,0,0,255))
    from sage_engine import gfx
    gfx.init(1,1)
    gfx.begin_frame()
    scene.render()
    buf = gfx.end_frame()
    assert buf[2] == 255
    gfx.shutdown()


def test_blur_effect():
    buf = bytearray([0, 0, 0, 255] * 9)
    # center pixel bright
    buf[4*4 + 2] = 255
    fx.apply('blur', buf, 3, 3)
    assert buf[2] < 255


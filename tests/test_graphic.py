from sage_engine.graphic.color import to_rgba
from sage_engine.gfx.runtime import GraphicRuntime
from sage_engine.graphic import fx


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


def test_blur_effect():
    buf = bytearray([0, 0, 0, 255] * 9)
    # center pixel bright
    buf[4*4 + 2] = 255
    fx.apply('blur', buf, 3, 3)
    assert buf[2] < 255


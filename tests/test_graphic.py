from sage_engine.graphic.color import to_rgba
from sage_engine.gfx.runtime import GraphicRuntime
from sage_engine.graphic import fx


def test_color_parsing():
    assert to_rgba((1, 2, 3)) == (1, 2, 3, 255)
    assert to_rgba('#01020304') == (1, 2, 3, 4)


def test_alpha_blend():
    rt = GraphicRuntime()
    rt.width = rt.height = 1
    rt.pitch = 4
    rt.buffer = bytearray(4)
    rt.begin_frame()
    rt.draw_rect(0, 0, 1, 1, (255, 0, 0, 255))
    rt.draw_rect(0, 0, 1, 1, (0, 0, 255, 128))
    # expect some blue mixed with red
    assert rt.buffer[0] > 0
    assert rt.buffer[2] < 255


def test_blur_effect():
    buf = bytearray([0, 0, 0, 255] * 9)
    # center pixel bright
    buf[4*4 + 2] = 255
    fx.apply('blur', buf, 3, 3)
    assert buf[2] < 255


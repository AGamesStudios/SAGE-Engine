from sage_engine.color import Color, parse_color, blend, Gradient


def test_parse_color_hex():
    c = parse_color("#112233")
    assert c.to_tuple() == (17, 34, 51, 255)


def test_blend():
    bg = Color(0, 0, 0)
    fg = Color(255, 0, 0, 128)
    out = blend(bg, fg)
    assert out.r > 0 and out.g == 0 and out.b == 0


def test_gradient():
    g = Gradient([(0.0, Color(0, 0, 0)), (1.0, Color(255, 255, 255))])
    mid = g.at(0.5)
    assert mid.r == 127 or mid.r == 128

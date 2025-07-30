import pytest
from sage_engine.render import mathops

def test_q8_mul():
    assert mathops.q8_mul(256, 256) == 256


def test_q8_lerp():
    assert mathops.q8_lerp(0, 256, 128) == 128


def test_blend_rgba_pm():
    # opaque red over opaque blue -> red
    r = 0xFFFF0000
    b = 0xFF0000FF
    assert mathops.blend_rgba_pm(b, r) == r

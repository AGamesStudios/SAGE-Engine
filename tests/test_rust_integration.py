import os
import pytest

from sage_engine.render import mathops
from sage_engine.render.optimizer import culling
from sage_engine.render.scheduler import PredictiveScheduler
from sage_engine.gfx.runtime import GraphicRuntime


def test_mathops_rust():
    assert mathops.q8_mul(256, 256) == 256
    assert mathops.q8_lerp(0, 256, 128) == 128
    r = 0xFFFF0000
    b = 0xFF0000FF
    assert mathops.blend_rgba_pm(b, r) == r


def test_culling_rust():
    objs = [("a", (0, 0, 5, 5)), ("b", (15, 15, 25, 25))]
    vis = culling.cull(objs, (10, 10, 20, 20))
    assert vis == ["b"]
    assert not culling.is_visible((0, 0, 1, 1), (5, 5, 10, 10))


def test_scheduler_rust():
    sched = PredictiveScheduler()
    for t in [0.02, 0.02, 0.02]:
        sched.record(t)
    assert sched.should_defer(10)


def test_gfx_rect_rust():
    rt = GraphicRuntime()
    rt.init(4, 4)
    rt.begin_frame()
    rt.draw_rect(0, 0, 2, 2, (255, 0, 0, 255))
    buf = rt.end_frame()
    assert buf[:1] != b""

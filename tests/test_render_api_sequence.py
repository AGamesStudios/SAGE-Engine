import pytest
from sage_engine import render
from sage_engine.render.api import RenderBackend
from sage_engine.render.context import RenderContext

class FakeBackend(RenderBackend):
    def __init__(self):
        self.calls = []
    def init(self, output_target):
        self.calls.append(("init", output_target))
    def begin_frame(self, handle=None):
        self.calls.append(("begin", handle))
    def draw_sprite(self, image, x, y, w, h, rotation=0.0, handle=None):
        self.calls.append(("sprite", handle))
    def draw_rect(self, x, y, w, h, color, handle=None):
        self.calls.append(("rect", x, y, w, h, color, handle))
    def end_frame(self, handle=None):
        self.calls.append(("end", handle))
    def present(self, buffer, handle=None):
        self.calls.append(("present", len(buffer), handle))
    def resize(self, width, height, handle=None):
        self.calls.append(("resize", width, height, handle))
    def set_viewport(self, x, y, w, h, handle=None):
        self.calls.append(("viewport", x, y, w, h, handle))
    def shutdown(self, handle=None):
        self.calls.append(("shutdown", handle))
    def create_context(self, output_target):
        self.init(output_target)
        return RenderContext(self, output_target)

def setup_backend(monkeypatch):
    fb = FakeBackend()
    monkeypatch.setattr(render, "_load_backend", lambda name: fb)
    render._backend = None
    render._context = None
    return fb

def test_render_api_sequence(monkeypatch):
    fb = setup_backend(monkeypatch)
    render.init("t")
    render.begin_frame()
    render.draw_rect(0, 0, 1, 1, (1, 2, 3, 4))
    render.end_frame()
    buf = bytearray(4)
    render.present(memoryview(buf))
    render.shutdown()
    assert fb.calls[2:6] == [
        ("begin", "t"),
        ("rect", 0, 0, 1, 1, (1, 2, 3, 4), "t"),
        ("end", "t"),
        ("present", 4, "t"),
    ]

def test_begin_frame_twice(monkeypatch):
    fb = setup_backend(monkeypatch)
    render.init("x")
    render.begin_frame()
    render.begin_frame()
    render.shutdown()
    begins = [c for c in fb.calls if c[0] == "begin"]
    assert len(begins) == 2

def test_present_without_begin(monkeypatch):
    fb = setup_backend(monkeypatch)
    render.init("p")
    buf = bytearray(8)
    render.present(memoryview(buf))
    render.shutdown()
    assert ("present", 8, "p") in fb.calls

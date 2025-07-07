import importlib
from tests.test_opengl_tilemap import _stub_gl


def _bbox(data):
    xs = data[0::2]
    ys = data[1::2]
    return min(xs), min(ys), max(xs)-min(xs), max(ys)-min(ys)


def test_flip_opengl_no_offset(monkeypatch):
    calls = {}
    _stub_gl(monkeypatch, calls)

    def capture(_target, _offset, _size, arr):
        calls.setdefault('verts', []).append([arr[i] for i in (0,1,4,5,8,9,12,13)])

    import OpenGL.GL as gl
    gl.glBufferSubData = capture
    gl.glGetUniformLocation = lambda *a, **k: 0
    gl.glUniform4f = lambda *a, **k: None
    gl.glBindBuffer = lambda *a, **k: None
    gl.glBindVertexArray = lambda *a, **k: None
    gl.glDrawArrays = lambda *a, **k: None
    gl.GL_ARRAY_BUFFER = 0
    gl.GL_TRIANGLE_FAN = 0
    import sys
    sys.modules['OpenGL.GL'].glGetUniformLocation = gl.glGetUniformLocation
    sys.modules['OpenGL.GL'].glUniform4f = gl.glUniform4f
    sys.modules['OpenGL.GL'].glBindBuffer = gl.glBindBuffer
    sys.modules['OpenGL.GL'].glBindVertexArray = gl.glBindVertexArray
    sys.modules['OpenGL.GL'].glDrawArrays = gl.glDrawArrays
    sys.modules['OpenGL.GL'].GL_ARRAY_BUFFER = 0
    sys.modules['OpenGL.GL'].GL_TRIANGLE_FAN = 0
    sys.modules['OpenGL.GL'].glBufferSubData = gl.glBufferSubData

    import engine.renderers.opengl.core as ogl_core
    ogl_core.glGetUniformLocation = gl.glGetUniformLocation
    ogl_core.glUniform4f = gl.glUniform4f
    ogl_core.glBindBuffer = gl.glBindBuffer
    ogl_core.glBindVertexArray = gl.glBindVertexArray
    ogl_core.glDrawArrays = gl.glDrawArrays
    ogl_core.GL_ARRAY_BUFFER = 0
    ogl_core.GL_TRIANGLE_FAN = 0
    ogl_core.glBufferSubData = gl.glBufferSubData

    ogl = importlib.import_module('engine.renderers.opengl_renderer')

    class DummyWidget:
        def __init__(self, *a, **k):
            pass
        def resize(self, w, h):
            pass
        def context(self):
            class Ctx:
                def isValid(self):
                    return False
            return Ctx()
        def update(self):
            pass

    monkeypatch.setattr(ogl, 'GLWidget', DummyWidget)
    r = ogl.OpenGLRenderer(width=32, height=32)
    r._program = 1

    class Obj:
        image = None
        width = 10
        height = 10
        scale_x = 1.0
        scale_y = 1.0
        x = 0
        y = 0
        pivot_x = 0.5
        pivot_y = 0.5
        angle = 0.0
        flip_x = False
        flip_y = False
        color = None
        alpha = 1.0
        def render_position(self, _cam):
            return self.x, self.y
        def render_scale(self, _cam):
            return 1.0
        def texture_coords(self, _cam):
            return [0,0,1,0,1,1,0,1]

    obj = Obj()
    r._draw_object(obj, None)
    no_flip = _bbox(calls['verts'][-1])
    obj.flip_x = True
    r._draw_object(obj, None)
    flip_x = _bbox(calls['verts'][-1])
    obj.flip_x = False
    obj.flip_y = True
    r._draw_object(obj, None)
    flip_y = _bbox(calls['verts'][-1])

    assert no_flip == flip_x == flip_y

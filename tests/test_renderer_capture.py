import types
import sys
import pytest

# Stub heavy dependencies
sys.modules['PyQt6'] = types.ModuleType('PyQt6')
sys.modules['PyQt6.QtCore'] = types.ModuleType('PyQt6.QtCore')
sys.modules['PyQt6.QtWidgets'] = types.ModuleType('PyQt6.QtWidgets')
qtgui = types.ModuleType('PyQt6.QtGui')
class DummyFormat:
    def setSamples(self, n):
        self.samples = n
    def setSwapInterval(self, n):
        self.interval = n
qtgui.QSurfaceFormat = DummyFormat
sys.modules['PyQt6.QtGui'] = qtgui
opengl_widgets = types.ModuleType('PyQt6.QtOpenGLWidgets')
opengl_widgets.QOpenGLWidget = object
sys.modules['PyQt6.QtOpenGLWidgets'] = opengl_widgets
sys.modules['PIL'] = types.ModuleType('PIL')
image_mod = types.ModuleType('PIL.Image')
image_mod.Image = type('Image', (), {})
image_mod.frombytes = lambda *a, **k: types.SimpleNamespace(size=(2, 2), transpose=lambda t: types.SimpleNamespace(size=(2, 2)))
image_mod.Transpose = types.SimpleNamespace(FLIP_TOP_BOTTOM=0)
sys.modules['PIL.Image'] = image_mod
sys.modules['PIL'].Image = image_mod

class DummyWidget:
    def __init__(self, parent=None, *, samples=4, vsync=None):
        self.samples = samples
        self.vsync = vsync
    def makeCurrent(self):
        pass
    def doneCurrent(self):
        pass
    def resize(self, w, h):
        self._w = w
        self._h = h
    def context(self):
        class Ctx:
            def isValid(self):
                return False
        return Ctx()
    def width(self):
        return 2
    def height(self):
        return 2
    def update(self):
        pass


def test_widget_config_and_grab(monkeypatch):
    gl_mod = types.ModuleType('OpenGL.GL')
    names = [
        'glEnable','glBlendFunc','glClearColor','glClear','glPushMatrix','glPopMatrix',
        'glTranslatef','glRotatef','glScalef','glBegin','glEnd','glVertex2f','glColor4f',
        'glTexCoord2f','glBindTexture','glTexParameteri','glTexImage2D','glGenTextures',
        'glLineWidth','glBufferSubData','glGetUniformLocation','glUniform4f','glUseProgram',
        'glBindBuffer','glBindVertexArray','glDrawArrays','glCopyTexImage2D','glDeleteTextures',
        'glViewport'
    ]
    for n in names:
        setattr(gl_mod, n, lambda *a, **k: None)
    gl_mod.glReadPixels = lambda x,y,w,h,fmt,tp: b"\xff\x00\x00\xff" * (w*h)
    consts = [
        'GL_BLEND','GL_SRC_ALPHA','GL_ONE_MINUS_SRC_ALPHA','GL_COLOR_BUFFER_BIT',
        'GL_TEXTURE_2D','GL_TEXTURE_MIN_FILTER','GL_TEXTURE_MAG_FILTER','GL_LINEAR','GL_NEAREST',
        'GL_QUADS','GL_LINES','GL_LINE_LOOP','GL_TRIANGLES','GL_RGBA','GL_UNSIGNED_BYTE',
        'GL_MULTISAMPLE','GL_LINE_SMOOTH','GL_ARRAY_BUFFER','GL_STATIC_DRAW','GL_FLOAT','GL_TRIANGLE_FAN'
    ]
    for c in consts:
        setattr(gl_mod, c, 0)
    monkeypatch.setitem(sys.modules, 'OpenGL', types.ModuleType('OpenGL'))
    monkeypatch.setitem(sys.modules, 'OpenGL.GL', gl_mod)
    shader_mod = types.ModuleType('engine.renderers.shader')
    shader_mod.Shader = object
    monkeypatch.setitem(sys.modules, 'engine.renderers.shader', shader_mod)
    if 'engine.renderers.opengl_renderer' in sys.modules:
        del sys.modules['engine.renderers.opengl_renderer']
    import importlib
    try:
        ogl = importlib.import_module('engine.renderers.opengl_renderer')
    except Exception as exc:  # pragma: no cover - optional dependency
        pytest.skip(f"opengl renderer unavailable: {exc}")
    monkeypatch.setattr(ogl, 'GLWidget', DummyWidget)
    r = ogl.OpenGLRenderer(width=2, height=2, samples=8, vsync=True)
    assert isinstance(r.widget, DummyWidget)
    assert r.widget.samples == 8
    assert r.widget.vsync is True
    img = r.grab_image()
    assert img.size == (2, 2)


def test_resize_to_zero(monkeypatch):
    gl_mod = types.ModuleType('OpenGL.GL')
    names = [
        'glEnable','glBlendFunc','glClearColor','glClear','glPushMatrix','glPopMatrix',
        'glTranslatef','glRotatef','glScalef','glBegin','glEnd','glVertex2f','glColor4f',
        'glTexCoord2f','glBindTexture','glTexParameteri','glTexImage2D','glGenTextures',
        'glLineWidth','glBufferSubData','glGetUniformLocation','glUniform4f','glUseProgram',
        'glBindBuffer','glBindVertexArray','glDrawArrays','glCopyTexImage2D','glDeleteTextures',
        'glViewport'
    ]
    for n in names:
        setattr(gl_mod, n, lambda *a, **k: None)
    gl_mod.glReadPixels = lambda x,y,w,h,fmt,tp: b""  # unused
    consts = [
        'GL_BLEND','GL_SRC_ALPHA','GL_ONE_MINUS_SRC_ALPHA','GL_COLOR_BUFFER_BIT',
        'GL_TEXTURE_2D','GL_TEXTURE_MIN_FILTER','GL_TEXTURE_MAG_FILTER','GL_LINEAR','GL_NEAREST',
        'GL_QUADS','GL_LINES','GL_LINE_LOOP','GL_TRIANGLES','GL_RGBA','GL_UNSIGNED_BYTE',
        'GL_MULTISAMPLE','GL_LINE_SMOOTH','GL_ARRAY_BUFFER','GL_STATIC_DRAW','GL_FLOAT','GL_TRIANGLE_FAN'
    ]
    for c in consts:
        setattr(gl_mod, c, 0)
    monkeypatch.setitem(sys.modules, 'OpenGL', types.ModuleType('OpenGL'))
    monkeypatch.setitem(sys.modules, 'OpenGL.GL', gl_mod)
    shader_mod = types.ModuleType('engine.renderers.shader')
    shader_mod.Shader = object
    monkeypatch.setitem(sys.modules, 'engine.renderers.shader', shader_mod)
    pil_mod = types.ModuleType('PIL')
    pil_image = types.ModuleType('PIL.Image')
    pil_image.Image = type('Image', (), {})
    pil_mod.Image = pil_image
    monkeypatch.setitem(sys.modules, 'PIL', pil_mod)
    monkeypatch.setitem(sys.modules, 'PIL.Image', pil_image)
    if 'engine.renderers.opengl_renderer' in sys.modules:
        del sys.modules['engine.renderers.opengl_renderer']
    import importlib
    ogl = importlib.import_module('engine.renderers.opengl_renderer')
    monkeypatch.setattr(ogl, 'GLWidget', DummyWidget)
    r = ogl.OpenGLRenderer(width=2, height=2)
    r.set_window_size(0, 0)
    # should not raise when painting with zero size
    r.paint()

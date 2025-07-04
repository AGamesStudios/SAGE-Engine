import sys
import types
from engine.entities.tile_map import TileMap


def _stub_gl(monkeypatch, calls):
    gl = types.ModuleType('OpenGL.GL')
    names = [
        'glGenTextures','glBindTexture','glTexParameteri','glTexImage2D','glBegin','glEnd',
        'glTexCoord2f','glVertex2f','glEnable','glBlendFunc','glDisable','glColor4f','glLineWidth',
        'glPushMatrix','glPopMatrix','glTranslatef','glUseProgram','glRotatef','glDeleteTextures',
        'glMatrixMode','glLoadIdentity','glOrtho'
    ]
    for n in names:
        if n == 'glGenTextures':
            setattr(gl, n, (lambda name: (lambda *a, **k: calls.setdefault(name, 0) or calls.__setitem__(name, calls[name]+1) or 1))(n))
        else:
            setattr(gl, n, (lambda name: (lambda *a, **k: calls.setdefault(name, 0) or calls.__setitem__(name, calls[name]+1)))(n))
    constants = [
        'GL_TEXTURE_2D','GL_TEXTURE_MIN_FILTER','GL_TEXTURE_MAG_FILTER','GL_NEAREST','GL_LINEAR','GL_RGBA','GL_UNSIGNED_BYTE','GL_QUADS',
        'GL_BLEND','GL_SRC_ALPHA','GL_ONE_MINUS_SRC_ALPHA','GL_MULTISAMPLE','GL_LINE_SMOOTH',
        'GL_LINES','GL_LINE_LOOP','GL_LINE_STRIP','GL_TRIANGLES','GL_TRIANGLE_FAN',
        'GL_PROJECTION','GL_MODELVIEW'
    ]
    for c in constants:
        setattr(gl, c, 0)
    monkeypatch.setitem(sys.modules, 'OpenGL', types.ModuleType('OpenGL'))
    monkeypatch.setitem(sys.modules, 'OpenGL.GL', gl)
    shader_mod = types.ModuleType('engine.renderers.shader')
    shader_mod.Shader = object
    monkeypatch.setitem(sys.modules, 'engine.renderers.shader', shader_mod)
    sys.modules['PyQt6'] = types.ModuleType('PyQt6')
    sys.modules['PyQt6.QtCore'] = types.ModuleType('PyQt6.QtCore')
    sys.modules['PyQt6.QtWidgets'] = types.ModuleType('PyQt6.QtWidgets')
    qtgui = types.ModuleType('PyQt6.QtGui')
    qtgui.QSurfaceFormat = type('DummyFormat', (), {'setSamples': lambda s,n: None, 'setSwapInterval': lambda s,n: None})
    sys.modules['PyQt6.QtGui'] = qtgui
    opengl_widgets = types.ModuleType('PyQt6.QtOpenGLWidgets')
    opengl_widgets.QOpenGLWidget = object
    sys.modules['PyQt6.QtOpenGLWidgets'] = opengl_widgets
    pil = types.ModuleType('PIL')
    img_mod = types.ModuleType('PIL.Image')
    class DummyImg:
        def __init__(self, size=(2,2)):
            self.size = size
        def transpose(self, t):
            return self
    img_mod.Image = DummyImg
    img_mod.Transpose = types.SimpleNamespace(FLIP_TOP_BOTTOM=0)
    img_mod.open = lambda p: DummyImg()
    img_mod.frombytes = lambda *a, **k: DummyImg()
    monkeypatch.setitem(sys.modules, 'PIL', pil)
    monkeypatch.setitem(sys.modules, 'PIL.Image', img_mod)


def test_tilemap_opengl(monkeypatch):
    calls = {}
    _stub_gl(monkeypatch, calls)
    import importlib
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
    tm = TileMap()
    tm.width = tm.height = 1
    tm.tile_width = tm.tile_height = 8
    tm.data = [1]
    tm.metadata = {'colors': {'1': '#ffffff'}}
    r = ogl.OpenGLRenderer(width=2, height=2)
    r._draw_map(tm)
    assert calls.get('glGenTextures') == 1


def test_draw_object_tilemap(monkeypatch):
    calls = {}
    _stub_gl(monkeypatch, calls)
    import importlib
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
    tm = TileMap()
    tm.width = tm.height = 1
    tm.tile_width = tm.tile_height = 8
    tm.data = [1]
    tm.metadata = {'colors': {'1': '#ffffff'}}
    r = ogl.OpenGLRenderer(width=2, height=2)
    r._program = 1
    r._draw_object(tm, None)
    assert calls.get('glUseProgram') == 1


def test_apply_projection_updates(monkeypatch):
    calls = {}
    _stub_gl(monkeypatch, calls)
    import importlib
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
    r = ogl.OpenGLRenderer(width=640, height=480)
    r._apply_projection(None)
    proj = r.get_projection()
    from engine.core import math2d
    assert proj == math2d.make_ortho(-320.0, 320.0, -240.0, 240.0)

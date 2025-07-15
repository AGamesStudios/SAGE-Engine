import sys
import types


def test_focus_policy_logs(monkeypatch, caplog):
    qtopen = types.ModuleType('PyQt6.QtOpenGLWidgets')
    class DummyWidget:
        def __init__(self, *a, **k):
            pass
        def setFormat(self, f):
            pass
        def setFocusPolicy(self, p):
            raise RuntimeError('bad')
    qtopen.QOpenGLWidget = DummyWidget
    qtgui = types.ModuleType('PyQt6.QtGui')
    class DummyFormat:
        def setSamples(self, s):
            pass
        def setSwapInterval(self, i):
            pass
    qtgui.QSurfaceFormat = lambda: DummyFormat()
    qtcore = types.ModuleType('PyQt6.QtCore')
    qtcore.Qt = types.SimpleNamespace(FocusPolicy=types.SimpleNamespace(StrongFocus=1))
    monkeypatch.setitem(sys.modules, 'PyQt6', types.ModuleType('PyQt6'))
    monkeypatch.setitem(sys.modules, 'PyQt6.QtOpenGLWidgets', qtopen)
    monkeypatch.setitem(sys.modules, 'PyQt6.QtGui', qtgui)
    monkeypatch.setitem(sys.modules, 'PyQt6.QtCore', qtcore)
    monkeypatch.setitem(sys.modules, 'OpenGL', types.ModuleType('OpenGL'))
    monkeypatch.setitem(sys.modules, 'OpenGL.GL', types.ModuleType('OpenGL.GL'))
    monkeypatch.setitem(sys.modules, 'OpenGL.GL.shaders', types.ModuleType('OpenGL.GL.shaders'))
    monkeypatch.setitem(sys.modules, 'PIL', types.ModuleType('PIL'))
    img_mod = types.ModuleType('PIL.Image')
    img_mod.Image = type('Image', (), {})
    monkeypatch.setitem(sys.modules, 'PIL.Image', img_mod)
    monkeypatch.setitem(sys.modules, 'engine.mesh_utils', types.ModuleType('engine.mesh_utils'))
    gl_mod = sys.modules['OpenGL.GL']
    gl_mod.glEnable = lambda *a, **k: None
    gl_mod.glBlendFunc = lambda *a, **k: None
    gl_mod.GL_BLEND = gl_mod.GL_SRC_ALPHA = gl_mod.GL_ONE_MINUS_SRC_ALPHA = 0
    gl_mod.GL_MULTISAMPLE = gl_mod.GL_LINE_SMOOTH = gl_mod.GL_TEXTURE_2D = 0
    gl_mod.GL_VERTEX_SHADER = gl_mod.GL_FRAGMENT_SHADER = 0
    gl_mod.glUseProgram = lambda *a, **k: None
    gl_mod.glGetUniformLocation = lambda *a, **k: 0
    gl_mod.glUniform1f = gl_mod.glUniform2f = gl_mod.glUniform3f = gl_mod.glUniform4f = lambda *a, **k: None
    shaders_mod = sys.modules['OpenGL.GL.shaders']
    shaders_mod.compileProgram = lambda *a, **k: 1
    shaders_mod.compileShader = lambda *a, **k: 1
    caplog.set_level('ERROR')
    import importlib.util
    import pathlib
    spec = importlib.util.spec_from_file_location(
        'gw', pathlib.Path('src/sage_engine/editor/qt/glwidget.py')
    )
    gw = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(gw)  # type: ignore[arg-type]
    gw.GLWidget()
    assert 'Failed to set FocusPolicy' in caplog.text

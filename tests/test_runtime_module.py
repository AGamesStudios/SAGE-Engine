import types
import sys
import importlib.machinery

# stub heavy dependencies so engine imports
qtcore = types.ModuleType('PyQt6.QtCore')
qtcore.Qt = object()
sys.modules.setdefault('PyQt6', types.ModuleType('PyQt6'))
sys.modules.setdefault('PyQt6.QtCore', qtcore)
sys.modules.setdefault('PyQt6.QtWidgets', types.ModuleType('PyQt6.QtWidgets'))
sys.modules.setdefault('PyQt6.QtOpenGLWidgets', types.ModuleType('PyQt6.QtOpenGLWidgets'))
sys.modules.setdefault('OpenGL', types.ModuleType('OpenGL'))
sys.modules.setdefault('OpenGL.GL', types.ModuleType('OpenGL.GL'))

render_mod = types.ModuleType('engine.renderers')
render_mod.Renderer = type('Renderer', (), {})
render_mod.Shader = object
render_mod.get_renderer = lambda name: None
render_mod.__spec__ = importlib.machinery.ModuleSpec('engine.renderers', None)
render_mod.__package__ = 'engine'
sys.modules['engine.renderers'] = render_mod
sys.modules['engine.renderers.shader'] = types.ModuleType('engine.renderers.shader')

mesh_mod = types.ModuleType('engine.mesh_utils')
mesh_mod.Mesh = type('Mesh', (), {})
mesh_mod.create_square_mesh = lambda *a, **k: None
mesh_mod.create_triangle_mesh = lambda *a, **k: None
mesh_mod.create_circle_mesh = lambda *a, **k: None
mesh_mod.__package__ = 'engine'
mesh_mod.__spec__ = importlib.machinery.ModuleSpec('engine.mesh_utils', None)
sys.modules['engine.mesh_utils'] = mesh_mod

sys.modules['PIL'] = types.ModuleType('PIL')
sys.modules['PIL.Image'] = types.ModuleType('PIL.Image')


def test_runtime_aliases():
    import engine
    import engine.runtime as er
    import sage_runtime as sr
    assert er.Engine is engine.Engine
    assert sr.Scene is engine.Scene


def test_runtime_entrypoints():
    called = []
    def fake_main(argv=None):
        called.append(argv)
    sys.modules['engine.core.engine'] = types.ModuleType('engine.core.engine')
    sys.modules['engine.core.engine'].main = fake_main
    import sage_runtime.__main__ as srm
    import engine.runtime.__main__ as erm
    srm.main(['one'])
    erm.main(['two'])
    assert called == [['one'], ['two']]


def test_versions_match_and_paint_dir():
    import engine
    import engine.runtime as er
    import sage_runtime as sr
    assert er.__version__ == engine.__version__
    assert sr.__version__ == engine.__version__
    assert 'paint' in dir(engine)

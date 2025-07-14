import os
import sys
import types
import pytest

sys.modules.setdefault('PIL', types.ModuleType('PIL'))
img_mod = types.ModuleType('PIL.Image')
img_mod.Image = type('Image', (), {})
sys.modules.setdefault('PIL.Image', img_mod)
sys.modules.setdefault('OpenGL', types.ModuleType('OpenGL'))
gl_mod = types.ModuleType('OpenGL.GL')
gl_mod.GL_VERTEX_SHADER = 0
gl_mod.GL_FRAGMENT_SHADER = 0
gl_mod.glUseProgram = lambda *a, **k: None
gl_mod.glGetUniformLocation = lambda *a, **k: 0
gl_mod.glUniform1f = gl_mod.glUniform2f = gl_mod.glUniform3f = gl_mod.glUniform4f = lambda *a, **k: None
sys.modules.setdefault('OpenGL.GL', gl_mod)
sh_mod = types.ModuleType('OpenGL.GL.shaders')
sh_mod.compileProgram = lambda *a, **k: 1
sh_mod.compileShader = lambda *a, **k: 1
sys.modules.setdefault('OpenGL.GL.shaders', sh_mod)
from engine.core.engine import Engine  # noqa: E402
from engine.core.scene_file import SceneFile  # noqa: E402


def test_sdl_renderer_run():
    try:
        import sdl2  # noqa: F401
    except Exception:
        pytest.xfail("SDL2 library not available")
    os.environ.setdefault("SDL_VIDEODRIVER", "dummy")
    scene = SceneFile.load('examples/Scenes/Scene1.sagescene').scene
    eng = Engine(scene=scene, renderer="sdl")
    eng.logic_active = True
    eng.step()
    eng.shutdown()
    eng.input.shutdown()
    eng.renderer.close()


def test_qt_input_backend_loop():
    try:
        from PyQt6.QtWidgets import QApplication
        from PyQt6.QtCore import QTimer
    except Exception:
        pytest.xfail("PyQt6 not available")

    try:
        app = QApplication.instance() or QApplication([])
    except Exception as exc:
        pytest.xfail(f"Qt platform unavailable: {exc}")
    scene = SceneFile.load('examples/Scenes/Scene1.sagescene').scene
    eng = Engine(scene=scene, renderer="null", input_backend="qt")

    def finish():
        eng.logic_active = True
        eng.step()
        eng.shutdown()
        eng.input.shutdown()
        eng.renderer.close()
        app.quit()

    QTimer.singleShot(0, finish)
    app.exec()

import os
import pytest
from engine.core.engine import Engine
from engine.core.scene_file import SceneFile


def test_sdl_renderer_run():
    try:
        import sdl2  # noqa: F401
    except Exception:
        pytest.skip("SDL2 library not available")
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
        pytest.skip("PyQt6 not available")

    app = QApplication.instance() or QApplication([])
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

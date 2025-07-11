import importlib.util
from pathlib import Path

from tests.test_viewport_sync import _setup_qt
from tests.test_opengl_tilemap import _stub_gl
from engine.core.project import Project
from engine.core.scenes.scene import Scene


def _load_viewport(monkeypatch):
    _stub_gl(monkeypatch, {})
    _setup_qt(monkeypatch)
    spec = importlib.util.spec_from_file_location(
        'viewport', Path('src/sage_editor/plugins/viewport.py')
    )
    viewport = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(viewport)
    return viewport


def test_load_and_save_project(tmp_path, monkeypatch):
    viewport = _load_viewport(monkeypatch)
    scene = Scene(with_defaults=False)
    proj = Project(scene=scene.to_dict())
    proj_path = tmp_path / 'proj.sageproject'
    proj.save(proj_path)

    win = viewport.EditorWindow()
    win.load_project(str(proj_path))
    assert win.project_path == str(proj_path)
    assert len(win.scene.objects) >= len(scene.objects)

    out = tmp_path / 'out.sageproject'
    win.save_project(str(out))
    loaded = Project.load(out)
    assert loaded.width == win.camera.width
    assert loaded.renderer == win.renderer_backend

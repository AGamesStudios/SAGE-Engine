import json
import sys
import types
import pytest
from engine.inputs import InputBackend  # noqa: E402


def _setup_stubs():
    sys.modules.setdefault('PIL', types.ModuleType('PIL'))
    img_mod = types.ModuleType('PIL.Image')
    img_mod.Image = type('Image', (), {})
    sys.modules['PIL.Image'] = img_mod
    sys.modules.setdefault('OpenGL', types.ModuleType('OpenGL'))
    sys.modules.setdefault('OpenGL.GL', types.ModuleType('OpenGL.GL'))
    sys.modules.setdefault('OpenGL.GL.shaders', types.ModuleType('OpenGL.GL.shaders'))
    gl_mod = sys.modules['OpenGL.GL']
    gl_mod.GL_VERTEX_SHADER = 0
    gl_mod.GL_FRAGMENT_SHADER = 0
    gl_mod.glUseProgram = lambda *a, **k: None
    gl_mod.glGetUniformLocation = lambda *a, **k: 0
    gl_mod.glUniform1f = gl_mod.glUniform2f = gl_mod.glUniform3f = gl_mod.glUniform4f = lambda *a, **k: None
    shaders_mod = sys.modules['OpenGL.GL.shaders']
    shaders_mod.compileProgram = lambda *a, **k: 1
    shaders_mod.compileShader = lambda *a, **k: 1
    sys.modules.setdefault('engine.mesh_utils', types.ModuleType('engine.mesh_utils'))
    sys.modules['engine.mesh_utils'].Mesh = type('Mesh', (), {})
    qtcore = types.ModuleType('PyQt6.QtCore')
    qtcore.Qt = object()
    qtcore.QObject = type('QObject', (), {})
    sys.modules.setdefault('PyQt6', types.ModuleType('PyQt6'))
    sys.modules['PyQt6.QtCore'] = qtcore
    sys.modules['PyQt6.QtWidgets'] = types.ModuleType('PyQt6.QtWidgets')


class DummyInput(InputBackend):
    def poll(self):
        pass
    def is_key_down(self, key: int) -> bool:
        return False
    def is_button_down(self, btn: int) -> bool:
        return False
    def shutdown(self) -> None:
        pass


@pytest.fixture(autouse=True)
def stubs(monkeypatch):
    _setup_stubs()
    monkeypatch.delitem(sys.modules, 'engine.core.engine', raising=False)
    yield


def test_save_and_load(tmp_path):
    from engine.entities.game_object import GameObject
    from engine.core.engine import Engine
    from engine.core.scenes.scene import Scene
    from engine.renderers.null_renderer import NullRenderer
    from engine.savegame import save_game, load_game

    scene = Scene(with_defaults=False)
    obj = GameObject(name="player")
    obj.x = 5
    obj.y = 10
    scene.add_object(obj)
    eng = Engine(scene=scene, renderer=NullRenderer, input_backend=DummyInput())
    save_path = tmp_path / "state.sagesave"
    save_game(eng, save_path)
    assert json.loads(save_path.read_text())["scene"]["objects"]
    new_eng = Engine(scene=Scene(with_defaults=False), renderer=NullRenderer, input_backend=DummyInput())
    load_game(new_eng, save_path)
    assert any(o.name == "player" for o in new_eng.scene.objects)

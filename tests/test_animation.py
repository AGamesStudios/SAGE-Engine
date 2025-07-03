import sys
import types

from engine.animation import Animation, Frame  # noqa: E402
from engine.formats import load_sageanimation, save_sageanimation  # noqa: E402


def test_save_and_load(tmp_path):
    path = tmp_path / 'walk.sageanimation'
    anim = Animation([Frame('a.png', 0.1), Frame('b.png', 0.2)], loop=False)
    save_sageanimation(anim, path)
    loaded = load_sageanimation(path)
    assert len(loaded.frames) == 2
    assert loaded.loop is False


def test_gameobject_animation(monkeypatch):
    # provide minimal stubs so GameObject imports without heavy deps
    pil_mod = types.ModuleType('PIL')
    img_mod = types.ModuleType('PIL.Image')
    class DummyImage:
        size = (32, 32)
        def convert(self, *a):
            return self
    img_mod.Image = DummyImage
    img_mod.open = lambda *a, **k: DummyImage()
    pil_mod.Image = img_mod
    monkeypatch.setitem(sys.modules, 'PIL', pil_mod)
    monkeypatch.setitem(sys.modules, 'PIL.Image', img_mod)
    monkeypatch.setitem(sys.modules, 'OpenGL', types.ModuleType('OpenGL'))
    gl_mod = types.ModuleType('OpenGL.GL')
    gl_mod.GL_VERTEX_SHADER = gl_mod.GL_FRAGMENT_SHADER = 0
    gl_mod.glUseProgram = lambda *a, **k: None
    gl_mod.glGetUniformLocation = lambda *a, **k: 0
    gl_mod.glUniform1f = gl_mod.glUniform2f = gl_mod.glUniform3f = gl_mod.glUniform4f = lambda *a, **k: None
    monkeypatch.setitem(sys.modules, 'OpenGL.GL', gl_mod)
    sh_mod = types.ModuleType('OpenGL.GL.shaders')
    sh_mod.compileProgram = lambda *a, **k: 1
    sh_mod.compileShader = lambda *a, **k: 1
    monkeypatch.setitem(sys.modules, 'OpenGL.GL.shaders', sh_mod)
    mesh_mod = types.ModuleType('engine.mesh_utils')
    mesh_mod.Mesh = type('Mesh', (), {})
    monkeypatch.setitem(sys.modules, 'engine.mesh_utils', mesh_mod)
    shader_mod = types.ModuleType('engine.renderers.shader')
    shader_mod.Shader = type('Shader', (), {'from_files': staticmethod(lambda v, f: None)})
    monkeypatch.setitem(sys.modules, 'engine.renderers.shader', shader_mod)

    from engine.entities.game_object import GameObject  # noqa: E402

    anim = Animation([Frame('a.png', 0.05), Frame('b.png', 0.05)])
    obj = GameObject(animation=anim)
    monkeypatch.setattr(GameObject, '_load_image', lambda self: None)
    obj.image_path = anim.image
    obj.update(0.05)
    assert obj.image_path == 'b.png'
    obj.update(0.05)
    assert obj.image_path == 'a.png'


def test_animation_speed_pause_reverse():
    anim = Animation([Frame('a.png', 0.1), Frame('b.png', 0.1)], loop=False)
    anim.speed = 2.0
    anim.update(0.05)
    assert anim.image == 'b.png'
    anim.pause()
    anim.update(1.0)
    assert anim.image == 'b.png'
    anim.play()
    anim.reverse = True
    anim.update(0.1)
    assert anim.image == 'a.png'


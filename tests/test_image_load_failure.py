import sys
import types
import pytest


def test_load_image_error(monkeypatch):
    import importlib
    monkeypatch.setitem(sys.modules, 'OpenGL', types.ModuleType('OpenGL'))
    gl_mod = types.ModuleType('OpenGL.GL')
    gl_mod.GL_VERTEX_SHADER = 0
    gl_mod.GL_FRAGMENT_SHADER = 0
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
    shader_mod.Shader = object
    monkeypatch.setitem(sys.modules, 'engine.renderers.shader', shader_mod)
    real_mod = importlib.import_module('engine.entities.game_object')
    monkeypatch.setitem(sys.modules, 'engine.entities.game_object', real_mod)
    real_mod.clear_image_cache()
    GameObject = real_mod.GameObject
    monkeypatch.setattr('PIL.Image.open', lambda p: (_ for _ in ()).throw(FileNotFoundError(p)))
    with pytest.raises(FileNotFoundError):
        GameObject(image_path='definitely_missing.png')


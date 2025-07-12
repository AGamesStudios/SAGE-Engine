import sys
import types

import engine.gizmos as gizmos

def _stub_gl(monkeypatch, calls):
    gl = types.ModuleType('OpenGL.GL')
    names = [
        'glBindTexture','glColor4f','glBegin','glEnd','glVertex2f','glLineWidth',
        'glPushMatrix','glPopMatrix','glTranslatef','glRotatef'
    ]
    for name in names:
        if name == 'glColor4f':
            setattr(gl, name, lambda r, g, b, a: calls.setdefault('color', []).append((r, g, b, a)))
        elif name == 'glLineWidth':
            setattr(gl, name, lambda w: calls.setdefault('width', []).append(w))
        elif name == 'glVertex2f':
            setattr(gl, name, lambda x, y: calls.setdefault('verts', []).append((x, y)))
        else:
            setattr(gl, name, lambda *a, **k: None)
    for const in [
        'GL_LINES','GL_TRIANGLES','GL_QUADS','GL_LINE_LOOP','GL_LINE_STRIP',
        'GL_TEXTURE_2D','GL_VERTEX_SHADER','GL_FRAGMENT_SHADER','GL_TRIANGLE_FAN'
    ]:
        setattr(gl, const, 0)
    monkeypatch.setitem(sys.modules, 'OpenGL', types.ModuleType('OpenGL'))
    monkeypatch.setitem(sys.modules, 'OpenGL.GL', gl)
    shader_mod = types.ModuleType('engine.renderers.shader')
    shader_mod.Shader = object
    monkeypatch.setitem(sys.modules, 'engine.renderers.shader', shader_mod)


def _load_gizmos(monkeypatch, calls):
    _stub_gl(monkeypatch, calls)
    pil = types.ModuleType('PIL')
    pil_image = types.ModuleType('PIL.Image')
    class DummyImage:
        pass
    pil_image.Image = DummyImage
    pil_image.open = lambda p: DummyImage()
    pil_image.frombytes = lambda *a, **k: DummyImage()
    pil.Image = pil_image
    monkeypatch.setitem(sys.modules, 'PIL', pil)
    monkeypatch.setitem(sys.modules, 'PIL.Image', pil_image)
    dummy_go_mod = types.ModuleType('engine.entities.game_object')
    dummy_go_mod.GameObject = type('GameObject', (), {})
    monkeypatch.setitem(sys.modules, 'engine.entities.game_object', dummy_go_mod)
    import importlib.util
    from pathlib import Path
    path = Path('src/engine/renderers/opengl/gizmos.py')
    spec = importlib.util.spec_from_file_location('ogl_gizmos', path)
    module = importlib.util.module_from_spec(spec)
    assert spec.loader
    spec.loader.exec_module(module)
    return module


def test_draw_basic_gizmo_colors(monkeypatch):
    calls = {}
    ogl_gizmos = _load_gizmos(monkeypatch, calls)
    g = gizmos.cross_gizmo(1, 2, size=5, color=(0.5, 0.5, 1, 0.8), thickness=3)
    ogl_gizmos.draw_basic_gizmo(None, g, None)
    assert calls['color'][0] == (0.5, 0.5, 1, 0.8)
    assert calls['width'][0] == 3


def test_polyline_gizmo(monkeypatch):
    calls = {}
    ogl_gizmos = _load_gizmos(monkeypatch, calls)
    points = [(0, 0), (1, 1), (2, 0)]
    g = gizmos.polyline_gizmo(points, color=(1, 0, 0, 1))
    ogl_gizmos.draw_basic_gizmo(None, g, None)
    assert calls['verts'] == [(0, 0), (1, 1), (2, 0)]
    assert calls['color'][0] == (1, 0, 0, 1)


def test_move_gizmo_resets_width(monkeypatch):
    calls = {}
    ogl_gizmos = _load_gizmos(monkeypatch, calls)
    Dummy = type('Obj', (), {'x': 0.0, 'y': 0.0, 'angle': 0.0})
    obj = Dummy()
    renderer = type('R', (), {'widget': None})()
    ogl_gizmos.draw_gizmo(renderer, obj, None, hover=None, dragging=None, mode='move', local=False)
    assert calls['width'][-1] == 1.0

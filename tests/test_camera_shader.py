import pytest
pytest.importorskip("OpenGL.GL")
from engine.core.camera import Camera
from engine.core.objects import object_to_dict, object_from_dict
try:
    from engine.renderers.shader import Shader
except Exception as exc:  # pragma: no cover - optional dependency
    pytest.skip(f"Shader module unavailable: {exc}", allow_module_level=True)


def test_camera_get_shader(tmp_path, monkeypatch):
    vert = tmp_path / "v.glsl"
    frag = tmp_path / "f.glsl"
    vert.write_text("v")
    frag.write_text("f")

    paths = []

    def fake_from_files(v, f):
        paths.append((v, f))
        return Shader("vs", "fs")

    monkeypatch.setattr(
        "engine.core.camera.Shader.from_files",
        staticmethod(fake_from_files),
    )

    cam = Camera(shader={"vertex": str(vert), "fragment": str(frag)})
    shader = cam.get_shader()

    assert paths == [(str(vert), str(frag))]
    assert isinstance(shader, Shader)


def test_camera_shader_serialization():
    cam = Camera(
        shader={"vertex": "v.glsl", "fragment": "f.glsl"},
        shader_uniforms={"foo": 1},
    )
    data = object_to_dict(cam)
    cam2 = object_from_dict(data)
    assert cam2.shader == {"vertex": "v.glsl", "fragment": "f.glsl"}
    assert cam2.shader_uniforms == {"foo": 1}


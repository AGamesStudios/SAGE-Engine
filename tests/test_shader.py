from engine.renderers.shader import Shader
from engine.core.game_object import GameObject


def test_shader_from_files(tmp_path, monkeypatch):
    vert = tmp_path / "v.glsl"
    frag = tmp_path / "f.glsl"
    vert.write_text("void main(){}")
    frag.write_text("void main(){}")
    compiled = {}

    def fake_compileShader(src, kind):
        compiled.setdefault("shaders", []).append(src)
        return len(compiled["shaders"])  # fake id

    def fake_compileProgram(v_id, f_id):
        compiled["program"] = (v_id, f_id)
        return 123

    monkeypatch.setattr("engine.renderers.shader.compileShader", fake_compileShader)
    monkeypatch.setattr("engine.renderers.shader.compileProgram", fake_compileProgram)

    shader = Shader.from_files(str(vert), str(frag))
    prog = shader.compile()

    assert prog == 123
    assert compiled["program"] == (1, 2)


def test_gameobject_get_shader(tmp_path, monkeypatch):
    vert = tmp_path / "v.glsl"
    frag = tmp_path / "f.glsl"
    vert.write_text("v")
    frag.write_text("f")

    paths = []

    def fake_from_files(v, f):
        paths.append((v, f))
        return Shader("vs", "fs")

    monkeypatch.setattr(
        "engine.core.game_object.Shader.from_files",
        staticmethod(fake_from_files),
    )

    obj = GameObject(shader={"vertex": str(vert), "fragment": str(frag)})
    shader = obj.get_shader()

    assert paths == [(str(vert), str(frag))]
    assert isinstance(shader, Shader)

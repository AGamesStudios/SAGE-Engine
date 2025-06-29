from engine.renderers.shader import Shader


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

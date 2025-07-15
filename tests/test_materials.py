from sage_engine import render, sprites
from sage_engine.render.shader import ShaderProgram
from sage_engine.render.material import Material


def test_material_group(tmp_path):
    backend = render.load_backend("headless")
    backend.create_device(16, 16)
    vert = tmp_path / "v.vert"
    frag = tmp_path / "f.frag"
    vert.write_text("void main(){}")
    frag.write_text("void main(){}")
    shader = ShaderProgram("t", vert.read_text(), frag.read_text())
    material = Material(shader)
    backend.register_shader(shader)
    sprites.clear()
    sprites.add(sprites.Sprite(0.0, 0.0, material=material))
    groups = sprites.collect_groups()
    backend.begin_frame()
    for mat, inst in groups:
        backend.set_material(mat)
        backend.draw_material_group(inst)
    backend.end_frame()
    assert backend.draw_calls == 1


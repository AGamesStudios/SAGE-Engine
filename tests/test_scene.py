from hypothesis import given, strategies as st
from sage_engine import scene


def setup_module():
    scene.reset()


@given(st.lists(st.sampled_from(["sprite", "camera"]), min_size=1, max_size=5))
def test_create_destroy(roles):
    edit = scene.scene.begin_edit()
    ids = [edit.create(role=r) for r in roles]
    scene.scene.apply(edit)
    assert len(scene.scene.roles) >= len(roles)

    edit = scene.scene.begin_edit()
    for i in ids:
        edit.destroy(i)
    scene.scene.apply(edit)
    scene.scene.commit()
    assert all(scene.scene.roles[i] is None for i in ids)


def test_serialize():
    edit = scene.scene.begin_edit()
    obj_id = edit.create(role="sprite", name="hero", x=10, y=5, tex_id=1)
    scene.scene.apply(edit)
    scene.scene.commit()
    data = scene.scene.serialize_object(obj_id)
    assert data["name"] == "hero"
    assert data["sprite"]["tex_id"] == 1
    assert obj_id in scene.scene.view.with_transform()


def test_scene_to_json():
    edit = scene.scene.begin_edit()
    edit.create(role="sprite", name="hero2", x=1)
    scene.scene.apply(edit)
    scene.scene.commit()
    data = scene.scene.to_json()
    assert any(obj["name"] == "hero2" for obj in data)

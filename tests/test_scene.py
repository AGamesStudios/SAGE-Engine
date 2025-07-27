from hypothesis import given, strategies as st
from sage_engine import world


def setup_module():
    world.reset()


@given(st.lists(st.sampled_from(["sprite", "camera"]), min_size=1, max_size=5))
def test_create_destroy(roles):
    edit = world.scene.begin_edit()
    ids = [edit.create(role=r) for r in roles]
    world.scene.apply(edit)
    assert len(world.scene.roles) >= len(roles)

    edit = world.scene.begin_edit()
    for i in ids:
        edit.destroy(i)
    world.scene.apply(edit)
    world.scene.commit()
    assert all(world.scene.roles[i] is None for i in ids)


def test_serialize():
    edit = world.scene.begin_edit()
    obj_id = edit.create(role="sprite", name="hero", x=10, y=5, tex_id=1)
    world.scene.apply(edit)
    world.scene.commit()
    data = world.scene.serialize_object(obj_id)
    assert data["name"] == "hero"
    assert data["sprite"]["tex_id"] == 1
    assert obj_id in world.scene.view.with_transform()


def test_scene_to_json():
    edit = world.scene.begin_edit()
    edit.create(role="sprite", name="hero2", x=1)
    world.scene.apply(edit)
    world.scene.commit()
    data = world.scene.to_json()
    assert any(obj["name"] == "hero2" for obj in data)

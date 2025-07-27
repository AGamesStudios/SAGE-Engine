from hypothesis import given, strategies as st
from sage_engine import scene


def setup_module():
    scene.reset()


@given(st.lists(st.sampled_from(['sprite', 'camera']), min_size=1, max_size=5))
def test_create_destroy(roles):
    edit = scene.scene.begin_edit()
    ids = [edit.create(r) for r in roles]
    scene.scene.apply(edit)
    assert len(scene.scene.objects) >= len(roles)
    edit = scene.scene.begin_edit()
    for i in ids:
        edit.destroy(i)
    scene.scene.apply(edit)
    assert all(scene.scene.objects[i].role is None for i in ids)

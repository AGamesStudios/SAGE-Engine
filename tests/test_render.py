from sage_engine import render, world, roles


def test_prepare_sort_flush():
    world.reset()
    edit = world.scene.begin_edit()
    edit.create(role="sprite", tex_id=1, x=0, y=0)
    edit.create(role="sprite", tex_id=2, x=1, y=0)
    world.scene.apply(edit)
    world.scene.commit()
    render.prepare(world.scene)
    assert render.prepared_data
    render.sort()
    assert render.prepared_data == sorted(render.prepared_data)
    render.flush()
    assert not render.prepared_data

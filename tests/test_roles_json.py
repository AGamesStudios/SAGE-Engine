from pathlib import Path
from sage_engine import core, roles, profiling, world
from sage_engine.scheduler import time


def setup_module():
    core.register('boot', time.boot)
    core.register('update', time.update)
    core.register('reset', world.reset)
    core.register('shutdown', world.reset)
    core.core_boot({})


def teardown_module():
    core.core_shutdown()


def test_roles_loaded():
    names = roles.registered_roles().keys()
    assert {'sprite', 'camera', 'player'}.issubset(names)


def test_role_update_called():
    world.reset()
    edit = world.scene.begin_edit()
    for r in ['sprite', 'camera', 'player']:
        edit.create(role=r)
    world.scene.apply(edit)
    world.scene.commit()
    before = dict(profiling.profile.role_calls)
    core.core_tick()
    for r in ['sprite', 'camera', 'player']:
        assert profiling.profile.role_calls[r] > before.get(r, 0)


def test_scene_edit_set():
    world.reset()
    edit = world.scene.begin_edit()
    obj = edit.create(role='sprite')
    world.scene.apply(edit)
    world.scene.commit()
    edit = world.scene.begin_edit()
    edit.set('sprite', obj, 'tex_id', 5)
    world.scene.apply(edit)
    world.scene.commit()
    assert world.scene.storage['sprite']['sprite']['tex_id'][0] == 5
    data = world.scene.serialize_object(obj)
    assert data['sprite']['tex_id'] == 5


def test_docs_generated():
    assert Path('docs/roles.md').exists()

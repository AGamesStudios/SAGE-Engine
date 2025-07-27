from pathlib import Path
from sage_engine import core, scene, roles, profiling, time


def setup_module():
    core.register('boot', time.boot)
    core.register('update', time.update)
    core.register('reset', scene.reset)
    core.register('shutdown', scene.reset)
    core.core_boot({})


def teardown_module():
    core.core_shutdown()


def test_roles_loaded():
    names = roles.registered_roles().keys()
    assert {'sprite', 'camera', 'player'}.issubset(names)


def test_role_update_called():
    scene.reset()
    edit = scene.scene.begin_edit()
    for r in ['sprite', 'camera', 'player']:
        edit.create(role=r)
    scene.scene.apply(edit)
    scene.scene.commit()
    before = dict(profiling.profile.role_calls)
    core.core_tick()
    for r in ['sprite', 'camera', 'player']:
        assert profiling.profile.role_calls[r] > before.get(r, 0)


def test_scene_edit_set():
    scene.reset()
    edit = scene.scene.begin_edit()
    obj = edit.create(role='sprite')
    scene.scene.apply(edit)
    scene.scene.commit()
    edit = scene.scene.begin_edit()
    edit.set('sprite', obj, 'tex_id', 5)
    scene.scene.apply(edit)
    scene.scene.commit()
    assert scene.scene.storage['sprite']['sprite']['tex_id'][0] == 5
    data = scene.scene.serialize_object(obj)
    assert data['sprite']['tex_id'] == 5


def test_docs_generated():
    assert Path('docs/roles.md').exists()

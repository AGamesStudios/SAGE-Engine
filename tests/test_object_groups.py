from sage_engine.objects import groups, runtime, Object
from sage_engine.objects.roles.interfaces import Role


class DummyRole(Role):
    def __init__(self) -> None:
        self.updated = 0
        self.rendered = 0

    def on_update(self, dt: float) -> None:
        self.updated += 1

    def on_render(self, ctx) -> None:
        self.rendered += 1


def setup_module() -> None:
    runtime.store.objects.clear()
    groups.groups.clear()


def test_group_basic_lifecycle() -> None:
    obj = Object(id="o1")
    obj.add_role("DummyRole", DummyRole())
    runtime.store.add_object(obj)
    gid = groups.create("test")
    groups.add(gid, obj.id)

    groups.disable_logic(gid)
    obj.update(0.1)
    assert obj.roles["DummyRole"].updated == 0

    groups.enable_logic(gid)
    obj.update(0.1)
    assert obj.roles["DummyRole"].updated == 1

    groups.hide_group(gid)
    obj.render(None)
    assert obj.roles["DummyRole"].rendered == 0

    groups.show_group(gid)
    obj.render(None)
    assert obj.roles["DummyRole"].rendered == 1

    groups.remove(gid, obj.id)
    assert obj.id not in groups.groups[gid].members
    groups.destroy(gid)


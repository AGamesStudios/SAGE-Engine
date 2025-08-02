from sage_engine.objects import groups, runtime, Object
from sage_engine.objects.groups import registry
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
    registry.GROUPS.clear()


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

    groups.disable_render(gid)
    obj.render(None)
    assert obj.roles["DummyRole"].rendered == 0

    groups.enable_render(gid)
    obj.render(None)
    assert obj.roles["DummyRole"].rendered == 1

    groups.remove(gid, obj.id)
    assert obj.id not in registry.GROUPS[gid].members
    groups.destroy(gid)


def test_dynamic_group_query() -> None:
    obj = Object(id="e1", world_id="Level1")
    obj.data["tags"] = ["enemy"]
    obj.add_role("DummyRole", DummyRole())
    runtime.store.add_object(obj)
    groups.register_object(obj)

    groups.add_dynamic("enemies", role="DummyRole", scene="Level1")
    registry.update_dynamic()
    assert obj.id in registry.GROUPS["enemies"].members


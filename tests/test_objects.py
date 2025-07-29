from sage_engine.objects import (
    Object, ObjectStore, ObjectBuilder, BlueprintSystem, runtime
)
from sage_engine.objects.roles.builtins import PhysicsBody, EnemyAI, Sprite


def test_manual_object_lifecycle():
    obj = Object(id="o1", name="hero")
    role = PhysicsBody(mass=5)
    obj.add_role("PhysicsBody", role)
    assert obj.has_role("PhysicsBody")
    obj.update(0.16)
    assert role.updated
    obj.remove_role("PhysicsBody")
    assert not obj.has_role("PhysicsBody")


def test_build_from_blueprint():
    bps = BlueprintSystem()
    bps.register({
        "name": "enemy_base",
        "roles": ["PhysicsBody"],
        "parameters": {"PhysicsBody": {"mass": 2}}
    })
    bps.register({
        "name": "enemy_tank",
        "extends": "enemy_base",
        "roles": ["EnemyAI"],
        "parameters": {"EnemyAI": {"difficulty": "hard"}}
    })
    store = ObjectStore()
    builder = ObjectBuilder(store, bps)
    obj = builder.build("enemy_tank", obj_id="tank1")
    assert obj.get_role("PhysicsBody").mass == 2
    assert obj.get_role("EnemyAI").difficulty == "hard"
    assert store.get_object_by_id("tank1") is obj


def test_store_update_render_query():
    store = runtime.store
    store.objects.clear()
    ctx = []
    bps = runtime.blueprints
    bps.register({"name": "sprite_only", "roles": ["Sprite"], "parameters": {"Sprite": {"image": "hero.png"}}})
    obj = runtime.builder().build("sprite_only", obj_id="s1")
    store.update(0.1)
    store.render(ctx)
    assert "render:hero.png" in ctx
    found = store.find_by_role("Sprite")
    assert obj in found

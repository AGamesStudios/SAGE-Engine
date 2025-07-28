from sage_engine.objects import ObjectStore, new, runtime


def test_create_get_patch_remove():
    store = ObjectStore()
    obj_id = store.create({"roles": ["Player"], "categories": {"Transform": {"x": 1}}})
    assert store.get(obj_id)["categories"]["Transform"]["x"] == 1
    store.patch(obj_id, {"categories": {"Transform": {"x": 2}}})
    assert store.get(obj_id)["categories"]["Transform"]["x"] == 2
    store.remove(obj_id)
    assert obj_id not in store.objects


def test_builder_spawn():
    store = ObjectStore()
    player_id = (new(store)
                 .role("Player")
                 .set("Transform", x=5)
                 .spawn())
    assert store.get(player_id)["categories"]["Transform"]["x"] == 5


def test_runtime_load():
    data = {
        "schema_version": "1.0",
        "objects": [
            {"roles": ["Player"], "categories": {"Transform": {"x": 7}}}
        ],
    }
    runtime.load(data)
    assert runtime.store.query_by_category("Transform")

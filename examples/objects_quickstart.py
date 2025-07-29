from sage_engine.objects import runtime

# register a simple blueprint
runtime.blueprints.register({"name": "Player", "roles": []})

# build an object instance from the blueprint
player = runtime.builder().build("Player", obj_id="player_01")

print(runtime.store.get_object_by_id("player_01"))

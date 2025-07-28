from sage_engine.objects import runtime, new

store = runtime.store

player_id = (
    new(store, "player_01")
    .role("Player")
    .set("Transform", x=100, y=64)
    .spawn()
)

print(store.get(player_id))

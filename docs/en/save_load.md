# Saving and Loading

Use `engine.save_game(engine, path)` to serialize the current state to a `.sagesave` file. Later you can restore it with `engine.load_game(engine, path)`.

Only engine and scene data are stored; custom objects must be registered via the plugin system so they can be recreated when loading.

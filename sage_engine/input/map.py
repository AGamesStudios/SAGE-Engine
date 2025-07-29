"""Simple action mapping storage."""
_action_map: dict[str, str] = {}

def map_action(name: str, key: str | None = None, gamepad_button: str | None = None) -> None:
    if key:
        _action_map[name] = key

def get_key(name: str) -> str | None:
    return _action_map.get(name)

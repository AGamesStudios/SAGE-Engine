_registry = {}

def register_widget(name: str, cls):
    _registry[name] = cls


def create(name: str, **kwargs):
    if name not in _registry:
        raise KeyError(f"Widget '{name}' not registered")
    return _registry[name](**kwargs)


def registered() -> dict:
    return dict(_registry)

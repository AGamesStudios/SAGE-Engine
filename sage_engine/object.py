"""Object system placeholder."""

_initialized = False


def init_object() -> None:
    global _initialized
    _initialized = True


def is_initialized() -> bool:
    return _initialized

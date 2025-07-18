"""UI subsystem placeholder."""

_initialized = False


def init_ui() -> None:
    global _initialized
    _initialized = True


def is_initialized() -> bool:
    return _initialized

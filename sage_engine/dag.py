"""Minimal DAG scheduler placeholder."""

_initialized = False


def init_dag() -> None:
    global _initialized
    _initialized = True


def is_initialized() -> bool:
    return _initialized

"""Minimal DAG scheduler placeholder."""

_initialized = False


def boot() -> None:
    """Initialise the DAG subsystem."""
    global _initialized
    _initialized = True


def reset() -> None:
    global _initialized
    _initialized = False


def destroy() -> None:
    reset()


def is_initialized() -> bool:
    return _initialized


# Backwards compatibility
init_dag = boot

__all__ = [
    "boot",
    "reset",
    "destroy",
    "is_initialized",
    "init_dag",
]

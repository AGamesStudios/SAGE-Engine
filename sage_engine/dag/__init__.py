"""Minimal DAG scheduler placeholder with task registry."""

_initialized = False
_tasks: dict[str, callable] = {}


def boot() -> None:
    """Initialise the DAG subsystem."""
    global _initialized
    _initialized = True


def register(name: str, func) -> None:
    """Register a callable task."""
    _tasks[name] = func


def run(name: str, *args, **kwargs):
    """Execute a registered task."""
    task = _tasks.get(name)
    if not task:
        raise KeyError(name)
    return task(*args, **kwargs)


def reset() -> None:
    global _initialized, _tasks
    _initialized = False
    _tasks.clear()


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
    "register",
    "run",
    "init_dag",
]

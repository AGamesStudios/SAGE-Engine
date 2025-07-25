from typing import Callable, Any

log: Callable[[str], None]
emit: Callable[[str], None]
on: Callable[[str, Callable[..., Any]], None]

counter = 0.0

def tick():
    log("tick event")  # noqa: F821

on("tick", tick)  # noqa: F821

def on_ready():
    log("timer ready")  # noqa: F821


def on_update(dt):
    global counter
    counter += dt
    if counter >= 1.0:
        emit("tick")  # noqa: F821
        counter -= 1.0

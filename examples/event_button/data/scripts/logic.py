from sage_engine.logic_api import on_ready, on_update, input, emit_event


def ready() -> None:
    print("Press SPACE to emit event")


def update(dt: float) -> None:
    if input.is_key_down("SPACE"):
        emit_event("button_press")


on_ready(ready)
on_update(update)

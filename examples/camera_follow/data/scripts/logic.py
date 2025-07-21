from sage_engine.logic_api import (
    on_ready,
    on_update,
    get_param,
    set_param,
    input,
)


def move(dt: float) -> None:
    dx = 0.0
    dy = 0.0
    if input.is_key_down("LEFT"):
        dx -= 100
    if input.is_key_down("RIGHT"):
        dx += 100
    if input.is_key_down("UP"):
        dy -= 100
    if input.is_key_down("DOWN"):
        dy += 100
    if dx or dy:
        x = (get_param("player", "x") or 0) + dx * dt
        y = (get_param("player", "y") or 0) + dy * dt
        set_param("player", "x", x)
        set_param("player", "y", y)
        set_param("MainCamera", "x", x)
        set_param("MainCamera", "y", y)


def ready_handler() -> None:
    print("Camera follow ready")


on_ready(ready_handler)
on_update(move)

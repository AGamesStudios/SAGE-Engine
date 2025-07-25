from sage_engine.logic_api import (
    on_ready,
    on_update,
    get_param,
    set_param,
)


TARGET = "Player"
CAMERA = "MainCamera"
OFFSET_X = 0.0
OFFSET_Y = 0.0


def update(dt: float) -> None:
    x = get_param(TARGET, "x") or 0
    y = get_param(TARGET, "y") or 0
    cam_x = x - OFFSET_X
    cam_y = y - OFFSET_Y
    set_param(CAMERA, "x", cam_x)
    set_param(CAMERA, "y", cam_y)
    print(f"[camera] move {cam_x:.1f} {cam_y:.1f}")


def ready_handler(_=None) -> None:
    print("[camera] follow init")


on_ready(ready_handler)
on_update(update)

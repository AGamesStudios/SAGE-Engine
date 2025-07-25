from sage_engine.logic_api import on_update, get_param, set_param
from sage_engine import input

BOX = "Box"
SPEED = 150


def update(dt: float) -> None:
    x = get_param(BOX, "x") or 0
    y = get_param(BOX, "y") or 0
    if input.input_key_down("left"):
        x -= SPEED * dt
    if input.input_key_down("right"):
        x += SPEED * dt
    if input.input_key_down("up"):
        y -= SPEED * dt
    if input.input_key_down("down"):
        y += SPEED * dt
    set_param(BOX, "x", x)
    set_param(BOX, "y", y)


on_update(update)

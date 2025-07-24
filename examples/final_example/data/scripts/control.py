from sage import on_update, input, set_param, get_param


def _move(obj_id: str, dx: float, dy: float) -> None:
    x = get_param(obj_id, "x") or 0
    y = get_param(obj_id, "y") or 0
    set_param(obj_id, "x", x + dx)
    set_param(obj_id, "y", y + dy)


@on_update
def update(dt: float) -> None:
    dx = 0.0
    dy = 0.0
    if input.is_key_down("LEFT"):
        dx -= 100 * dt
    if input.is_key_down("RIGHT"):
        dx += 100 * dt
    if input.is_key_down("UP"):
        dy -= 100 * dt
    if input.is_key_down("DOWN"):
        dy += 100 * dt
    if dx or dy:
        _move("player", dx, dy)

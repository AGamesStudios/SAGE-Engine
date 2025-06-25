from __future__ import annotations

import math

try:
    from numba import njit
except Exception:  # pragma: no cover - numba optional
    def njit(*args, **kwargs):
        def wrap(func):
            return func
        return wrap

@njit(cache=True)
def angle_to_quat(angle: float) -> tuple[float, float, float, float]:
    rad = math.radians(angle) / 2.0
    return 0.0, 0.0, math.sin(rad), math.cos(rad)

@njit(cache=True)
def quat_to_angle(z: float, w: float) -> float:
    return math.degrees(2.0 * math.atan2(z, w))

@njit(cache=True)
def calc_rect(x: float, y: float, width: float, height: float, pivot_x: float,
              pivot_y: float, scale_x: float, scale_y: float,
              angle: float, units_per_meter: float, y_up: bool) -> tuple[float, float, float, float]:
    px = width * pivot_x
    py = height * pivot_y
    rad = math.radians(angle)
    ca = math.cos(rad)
    sa = math.sin(rad)
    sign = 1.0 if y_up else -1.0
    tx = x * units_per_meter
    ty = y * units_per_meter * sign
    xs = [0.0, 0.0, 0.0, 0.0]
    ys = [0.0, 0.0, 0.0, 0.0]
    corners = ((-px, -py), (width - px, -py), (width - px, height - py), (-px, height - py))
    for i in range(4):
        cx = corners[i][0] * scale_x
        cy = corners[i][1] * scale_y
        rx = cx * ca - cy * sa
        ry = cx * sa + cy * ca
        xs[i] = tx + rx
        ys[i] = ty + ry
    left = min(xs)
    right = max(xs)
    bottom = min(ys)
    top = max(ys)
    return left, bottom, right - left, top - bottom

@njit(cache=True)
def calc_matrix(x: float, y: float, width: float, height: float, pivot_x: float,
                pivot_y: float, scale_x: float, scale_y: float,
                angle: float, units_per_meter: float, y_up: bool):
    ang = math.radians(angle)
    ca = math.cos(ang)
    sa = math.sin(ang)
    px = width * pivot_x
    py = height * pivot_y
    m00 = ca * scale_x
    m01 = -sa * scale_y
    m10 = sa * scale_x
    m11 = ca * scale_y
    sign = 1.0 if y_up else -1.0
    tx = x * units_per_meter + px - (m00 * px + m01 * py)
    ty = y * units_per_meter * sign + py - (m10 * px + m11 * py)
    return (
        m00, m10, 0.0, 0.0,
        m01, m11, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        tx,  ty,  0.0, 1.0,
    )

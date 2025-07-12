
import math

try:
    from numba import njit  # type: ignore[import-not-found]
except Exception:  # pragma: no cover - numba optional
    def njit(*args, **kwargs):
        def wrap(func):
            return func
        return wrap

__all__ = [
    "angle_to_quat",
    "quat_to_angle",
    "calc_rect",
    "calc_matrix",
    "make_transform",
    "transform_point",
    "make_ortho",
    "multiply_matrix",
    "decompose_matrix",
    "normalize_angle",
    "set_max_angle",
    "get_max_angle",
]


@njit(cache=True)
def angle_to_quat(angle: float) -> tuple[float, float, float, float]:
    rad = math.radians(angle) / 2.0
    return 0.0, 0.0, math.sin(rad), math.cos(rad)


@njit(cache=True)
def quat_to_angle(z: float, w: float) -> float:
    return math.degrees(2.0 * math.atan2(z, w))


# maximum angle used by :func:`normalize_angle`
_MAX_ANGLE = 360.0


def set_max_angle(value: float) -> None:
    """Set the maximum angle used for wrapping."""
    global _MAX_ANGLE
    _MAX_ANGLE = 360.0 if value <= 0 else float(value)


def get_max_angle() -> float:
    """Return the current ``MAX_ANGLE`` value."""
    return _MAX_ANGLE


def normalize_angle(angle: float) -> float:
    """Return *angle* wrapped into the ``0``–``MAX_ANGLE`` range."""
    value = angle % _MAX_ANGLE
    if value < 0:
        value += _MAX_ANGLE
    return value


@njit(cache=True)
def calc_rect(
    x: float,
    y: float,
    width: float,
    height: float,
    pivot_x: float,
    pivot_y: float,
    scale_x: float,
    scale_y: float,
    angle: float,
    units_per_meter: float,
    y_up: bool,
) -> tuple[float, float, float, float]:
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
def calc_matrix(
    x: float,
    y: float,
    width: float,
    height: float,
    pivot_x: float,
    pivot_y: float,
    scale_x: float,
    scale_y: float,
    angle: float,
    units_per_meter: float,
    y_up: bool,
):
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


def make_transform(
    x: float = 0.0,
    y: float = 0.0,
    scale_x: float = 1.0,
    scale_y: float = 1.0,
    angle: float = 0.0,
    pivot_x: float = 0.0,
    pivot_y: float = 0.0,
) -> list[float]:
    """Return a 3x3 column-major 2D transform matrix."""
    ang = math.radians(angle)
    ca = math.cos(ang)
    sa = math.sin(ang)
    tx = x - (pivot_x * ca - pivot_y * sa) * scale_x
    ty = y - (pivot_x * sa + pivot_y * ca) * scale_y
    return [
        ca * scale_x, sa * scale_x, 0.0,
        -sa * scale_y, ca * scale_y, 0.0,
        tx, ty, 1.0,
    ]


def transform_point(mat: list[float], x: float, y: float) -> tuple[float, float]:
    """Transform ``(x, y)`` by ``mat`` produced by :func:`make_transform`."""
    nx = mat[0] * x + mat[3] * y + mat[6]
    ny = mat[1] * x + mat[4] * y + mat[7]
    return nx, ny


def make_ortho(left: float, right: float, bottom: float, top: float) -> list[float]:
    """Return a column-major 3x3 orthographic projection matrix."""
    rl = right - left
    tb = top - bottom
    if rl == 0 or tb == 0:
        raise ValueError("Invalid projection size")
    return [
        2.0 / rl, 0.0, 0.0,
        0.0, 2.0 / tb, 0.0,
        -(right + left) / rl, -(top + bottom) / tb, 1.0,
    ]


def multiply_matrix(a: list[float], b: list[float]) -> list[float]:
    """Return ``a * b`` for two column-major 3x3 matrices."""
    return [
        a[0] * b[0] + a[3] * b[1] + a[6] * b[2],
        a[1] * b[0] + a[4] * b[1] + a[7] * b[2],
        0.0,
        a[0] * b[3] + a[3] * b[4] + a[6] * b[5],
        a[1] * b[3] + a[4] * b[4] + a[7] * b[5],
        0.0,
        a[0] * b[6] + a[3] * b[7] + a[6] * b[8],
        a[1] * b[6] + a[4] * b[7] + a[7] * b[8],
        1.0,
    ]


def decompose_matrix(mat: list[float]) -> tuple[float, float, float, float, float]:
    """Return ``(x, y, sx, sy, angle)`` from ``mat``."""
    sx = math.hypot(mat[0], mat[1])
    sy = math.hypot(mat[3], mat[4])
    if sx == 0 or sy == 0:
        raise ValueError("Invalid matrix scale")
    angle = math.degrees(math.atan2(mat[1] / sx, mat[0] / sx))
    return mat[6], mat[7], sx, sy, angle


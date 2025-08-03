from __future__ import annotations

from ..core import expose
from ..logger import logger
from ..graphics.camera3d import Camera3D

_active: Camera3D | None = None

def set_active_camera(camera: Camera3D | None) -> None:
    """Register ``camera`` as the active 3D camera."""
    global _active
    _active = camera
    if camera is not None:
        logger.info("[camera] Active camera set successfully", tag="camera")

def get_active_camera() -> Camera3D | None:
    """Return currently active 3D camera or ``None``."""
    return _active

expose("camera3d", {
    "set_active_camera": set_active_camera,
    "get_active_camera": get_active_camera,
})

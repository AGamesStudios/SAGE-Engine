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
    else:
        logger.warn("[camera] Active camera cleared", tag="camera")

def get_active_camera() -> Camera3D | None:
    """Return currently active 3D camera or ``None``."""
    cam = _active
    if cam is None:
        logger.debug("[camera] get_active_camera -> None", tag="camera")
    else:
        logger.debug(
            "[camera] get_active_camera -> pos=%s look=%s", cam.position, cam.look_at, tag="camera"
        )
    return cam

expose("camera3d", {
    "set_active_camera": set_active_camera,
    "get_active_camera": get_active_camera,
})

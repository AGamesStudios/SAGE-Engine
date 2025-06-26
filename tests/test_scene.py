import pytest
from engine.core.scene import Scene


def test_ensure_active_camera_creates_camera():
    scene = Scene()
    cam = scene.ensure_active_camera(800, 600)
    assert cam in scene.objects
    assert scene.get_active_camera() == cam


def test_ensure_active_camera_returns_existing():
    scene = Scene()
    cam1 = scene.ensure_active_camera(800, 600)
    cam2 = scene.ensure_active_camera(800, 600)
    assert cam1 is cam2

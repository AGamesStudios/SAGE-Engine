import pytest
from sage_object import object_from_dict, InvalidRoleError, get_available_roles
from sage_engine import render


def test_role_defaults_and_serialisation():
    obj = object_from_dict({"id": "cam1", "name": "MainCam", "role": "Camera", "zoom": 1.5})
    assert obj.params["bounds"] == [0, 0, 1920, 1080]
    assert obj.params["zoom"] == 1.5
    data = obj.to_dict()
    assert data["role"] == "Camera"
    assert data["zoom"] == 1.5


def test_invalid_role():
    with pytest.raises(InvalidRoleError):
        object_from_dict({"role": "Spritte"})


def test_render_integration():
    cam = object_from_dict({"name": "c", "role": "Camera"})
    sprite = object_from_dict({"name": "s", "role": "Sprite", "image": "hero.png"})
    assert render.render_object(cam).startswith("Camera")
    assert render.render_object(sprite).startswith("Sprite")
    assert "Camera" in get_available_roles()

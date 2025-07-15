from sage_engine.entities.game_object import GameObject


def test_empty_role_clears_graphics():
    obj = GameObject(image_path="img.png", shape="square")
    obj.set_role("empty")
    assert obj.image_path == ""
    assert obj.shape is None
    assert obj.width == 0 and obj.height == 0


def test_camera_role_defaults():
    obj = GameObject()
    obj.set_role("camera")
    assert obj.image_path == ""
    assert obj.shape is None
    assert obj.metadata["width"] == 640
    assert obj.metadata["height"] == 480

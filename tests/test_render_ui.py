from sage_object import object_from_dict
from sage_engine.render import render_scene
from sage_engine.ui import render_ui


def test_render_scene_batches():
    objs = [
        object_from_dict({"role": "Sprite", "image": "atlas.png", "layer": 0}),
        object_from_dict({"role": "Sprite", "image": "atlas.png", "layer": 1}),
        object_from_dict({"role": "Sprite", "image": "other.png", "layer": 2}),
    ]
    calls = render_scene(objs)
    assert "SpriteBatch image=atlas.png count=2" in calls
    assert "SpriteBatch image=other.png count=1" in calls


def test_render_ui_collects():
    objs = [
        object_from_dict({"role": "UI", "text": "Hello"}),
        object_from_dict({"role": "Sprite", "image": "img.png"}),
    ]
    calls = render_ui(objs)
    assert calls == ["UI text=Hello"]

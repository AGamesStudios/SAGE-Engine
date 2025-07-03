from engine.core.scenes.scene import Scene
from engine.entities.game_object import GameObject


def test_scene_aggregates_public_variables_only():
    scene = Scene(with_defaults=False)
    obj = GameObject()
    obj.add_variable("secret", 1, public=False)
    obj.add_variable("score", 2, public=True)
    scene.add_object(obj)
    es = scene.build_event_system()
    assert "score" in es.variables
    assert "secret" not in es.variables


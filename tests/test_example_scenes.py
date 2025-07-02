from engine.core.scene_file import SceneFile


def test_example_scenes_load():
    for name in [
        'examples/Scenes/Animation.sagescene',
        'examples/Scenes/Audio.sagescene',
        'examples/Scenes/Logic.sagescene',
    ]:
        scene = SceneFile.load(name).scene
        assert scene.objects

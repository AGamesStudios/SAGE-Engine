from sage_engine.core.scene_file import SceneFile


def test_example_scenes_load():
    for name in [
        'examples/Scenes/Animation.sagescene',
        'examples/Scenes/Audio.sagescene',
        'examples/Scenes/Logic.sagescene',
        'examples/Scenes/Map.sagescene',
        'examples/Scenes/Physics.sagescene',
    ]:
        scene = SceneFile.load(name).scene
        assert scene.objects


def test_advanced_project_scenes():
    from sage_engine.core.project import Project
    proj = Project.load('examples/advanced.sageproject')
    for node in proj.scene_graph.get('nodes', {}).values():
        path = 'examples/' + node['scene_file']
        scene = SceneFile.load(path).scene
        assert scene.objects

from engine.core.project import Project
from engine.core.scenes.scene import Scene


def test_project_save_load(tmp_path):
    scene = Scene(with_defaults=False)
    proj = Project(scene=scene.to_dict())
    proj_path = tmp_path / "proj.sageproject"
    proj.save(proj_path)
    loaded = Project.load(proj_path)
    assert loaded.width == proj.width
    assert loaded.height == proj.height
    assert loaded.scene

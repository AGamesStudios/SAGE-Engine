from engine.core.project import Project


def test_blank_project_load():
    proj = Project.load('examples/blank.sageproject')
    assert proj.width == 640
    assert isinstance(proj.scene.get('objects'), list)
    assert len(proj.scene['objects']) >= 2

from engine.core.project import Project


def test_blank_project_load():
    proj = Project.load('examples/blank.sageproject')
    assert proj.width == 640
    assert proj.scene == {}

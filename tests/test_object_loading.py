import json
from pathlib import Path

import pytest

from sage_engine.resource import ResourceManager
from sage_object import InvalidRoleError


def test_load_objects(tmp_path: Path) -> None:
    file = tmp_path / "cam.sage_object"
    file.write_text(json.dumps({"role": "Camera", "zoom": 2.0}))
    mgr = ResourceManager()
    objs = mgr.load_objects(str(file))
    assert len(objs) == 1
    cam = objs[0]
    assert cam.params["zoom"] == 2.0
    assert cam.params["bounds"] == [0, 0, 1920, 1080]


def test_load_objects_empty(tmp_path: Path) -> None:
    file = tmp_path / "empty.sage_object"
    file.write_text("[]")
    mgr = ResourceManager()
    objs = mgr.load_objects(str(file))
    assert objs == []


def test_load_objects_invalid_role(tmp_path: Path) -> None:
    file = tmp_path / "bad.sage_object"
    file.write_text(json.dumps({"role": "Unknown"}))
    mgr = ResourceManager()
    with pytest.raises(InvalidRoleError):
        mgr.load_objects(str(file))


def test_load_all_objects(tmp_path: Path) -> None:
    folder = tmp_path / "objs"
    folder.mkdir()
    (folder / "s.sage_object").write_text(json.dumps({"role": "Sprite"}))
    mgr = ResourceManager()
    objs = mgr.load_all_objects(str(folder))
    assert len(objs) == 1
    assert objs[0].role == "Sprite"

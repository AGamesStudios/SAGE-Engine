import json
from pathlib import Path

from sage_engine import api
from sage_engine.blueprint import load


def test_blueprint_migration(tmp_path: Path):
    data = {
        "schema_version": 1,
        "sprite": "hero.png",
        "objects": []
    }
    p = tmp_path / "old.json"
    p.write_text(json.dumps(data), encoding="utf8")
    bp = load(p)
    assert bp.schema_version == 2
    assert bp.meta
    assert bp.objects == []
    assert isinstance(bp, api.Blueprint)


def test_api_stable():
    assert hasattr(api.scene, "begin_edit")
    assert api.Blueprint.__stable_api__ == True


def test_scene_migration():
    data = {"engine_version": 1, "entities": []}
    ver, migrated = api.compat.migrate("scene", 1, 2, dict(data))
    assert ver == 2
    assert "objects" in migrated


def test_flow_migration():
    data = {"schema_version": 1, "variables": {"hp": 5}}
    ver, migrated = api.compat.migrate("flowscript", 1, 2, dict(data))
    assert ver == 2
    assert migrated["variables"]["health"] == 5

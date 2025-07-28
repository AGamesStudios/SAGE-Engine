import json
from pathlib import Path

from sage_engine import api
from sage_engine.blueprint import load


def test_blueprint_migration(tmp_path: Path):
    data = {
        "schema_version": "0.9",
        "sprite": "hero.png",
        "objects": []
    }
    p = tmp_path / "old.json"
    p.write_text(json.dumps(data), encoding="utf8")
    bp = load(p)
    assert bp.schema_version == "1.0"
    assert bp.meta
    assert bp.objects == []
    assert isinstance(bp, api.Blueprint)


def test_api_stable():
    assert hasattr(api.scene, "begin_edit")
    assert api.Blueprint.__stable_api__ == True

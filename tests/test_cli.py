from pathlib import Path
import json
from sage_engine.devtools import cli


def test_blueprint_migrate(tmp_path):
    bp = tmp_path / "bp.json"
    data = {"schema_version": 1, "sprite": {"tex": 1}}
    bp.write_text(json.dumps(data))
    cli.blueprint_migrate(bp)
    new_data = json.loads(bp.read_text())
    assert "renderable" in new_data


def test_compat_check(tmp_path, capsys):
    sc = tmp_path / "scene.json"
    data = {"engine_version": 1, "entities": []}
    sc.write_text(json.dumps(data))
    cli.compat_check(sc)
    out = capsys.readouterr().out
    assert "Needs migration" in out

from pathlib import Path
from sage_engine.blueprint import load


def test_blueprint_load(tmp_path):
    data = {
        "meta": {"origin": "test"},
        "objects": [{"objectName": "old", "deprecatedField": 1}],
    }
    p = tmp_path / "bp.json"
    p.write_text(__import__('json').dumps(data), encoding='utf8')
    bp = load(p)
    assert bp.objects[0]["name"] == "old"
    assert "deprecatedField" not in bp.objects[0]
    assert bp.meta.origin == "test"

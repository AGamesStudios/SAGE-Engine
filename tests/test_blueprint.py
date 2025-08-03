from pathlib import Path
from sage_engine.blueprint import load


def test_blueprint_load(tmp_path):
    data = {
        "meta": {"origin": "test"},
        "objects": []
    }
    p = tmp_path / "bp.json"
    p.write_text(__import__('json').dumps(data), encoding='utf8')
    bp = load(p)
    assert bp.objects == []
    assert bp.meta.origin == "test"

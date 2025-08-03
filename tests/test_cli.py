import json
from sage_engine.devtools import cli


def test_debug_stats(capsys):
    cli.debug_stats()
    out = capsys.readouterr().out
    data = json.loads(out)
    assert "sprites_drawn" in data


def test_transform_info(capsys):
    cli.transform_info()
    out = capsys.readouterr().out
    data = json.loads(out)
    assert "total_objects" in data and "visible_objects" in data


def test_info_versionless(capsys):
    cli.main(["info"])
    out = capsys.readouterr().out.lower()
    assert "versionless: true" in out
    assert "migrated_files:" in out
    assert "format_revision:" in out

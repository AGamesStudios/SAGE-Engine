import json
from sage_engine.devtools import cli


def test_debug_stats(capsys):
    cli.debug_stats()
    out = capsys.readouterr().out
    data = json.loads(out)
    assert "sprites_drawn" in data

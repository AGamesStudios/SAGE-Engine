import re
from sage_engine.devtools import cli


def test_info_runner(capsys):
    cli.main(["info", "runner"])
    out = capsys.readouterr().out
    assert "world:" in out
    assert "objects:" in out
    assert "roles:" in out

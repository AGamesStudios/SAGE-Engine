from sage_engine.devtools import cli


def test_info_render(capsys):
    cli.main(["info", "render"])
    assert "sprites_drawn" in capsys.readouterr().out


def test_info_world(capsys):
    cli.main(["info", "world"])
    out = capsys.readouterr().out
    assert "objects:" in out

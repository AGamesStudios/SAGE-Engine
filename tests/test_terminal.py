from sage_engine.terminal import SageTerminal


def test_terminal_help(capsys):
    term = SageTerminal()
    term.onecmd("help")
    out = capsys.readouterr().out
    assert "run" in out

import sage


def test_sage_cli_commands(monkeypatch):
    called = []
    monkeypatch.setitem(sage._COMMANDS, "build", lambda a: called.append("build"))
    assert sage.main(["build"]) == 0
    assert called == ["build"]

from sage_engine.utils.reload_watcher import check_reload


def test_check_reload(tmp_path, monkeypatch):
    flag = tmp_path / "reload.flag"
    flag.touch()
    called = False

    def fake_reload(mod):
        nonlocal called
        called = True

    monkeypatch.setattr("importlib.import_module", lambda name: object())
    monkeypatch.setattr("importlib.reload", fake_reload)
    check_reload(flag)
    assert not flag.exists()
    assert called

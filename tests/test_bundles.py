import sage
from engine import bundles, adaptors


def test_load_bundle():
    data = bundles.load_bundle("mobile")
    assert "adaptors" in data


def test_cli_build_bundle(monkeypatch):
    called = {}
    monkeypatch.setattr(adaptors, "load_adaptors", lambda names=None: called.setdefault("names", names))
    monkeypatch.setattr(bundles, "load_bundle", lambda name: {"adaptors": {"list": ["render"]}})
    assert sage.main(["build", "--bundle", "mobile"]) == 0
    assert called["names"] == ["render"]



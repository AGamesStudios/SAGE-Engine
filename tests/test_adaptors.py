from importlib import metadata
import engine.adaptors as adaptors


class _EP:
    def __init__(self, name, called):
        self.name = name
        self._called = called
    def load(self):
        def reg():
            self._called.append(self.name)
        return reg


class _EPS(list):
    def __init__(self, group, called):
        super().__init__([_EP("a1", called), _EP("a2", called)])
        self.group = group
    def select(self, *, group):
        return self if group == self.group else []


def test_adaptor_loading(monkeypatch):
    called = []
    eps = _EPS("sage_adaptor", called)
    monkeypatch.setattr(metadata, "entry_points", lambda: eps)
    adaptors._LOADED = set()
    adaptors.load_adaptors()
    assert set(called) == {"a1", "a2"}

    # loading again with selection should skip already loaded adaptors
    adaptors.load_adaptors(["a2"])
    assert called.count("a2") == 1


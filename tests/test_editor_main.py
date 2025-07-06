from sage_editor import main, Editor


def test_main_no_window(monkeypatch):
    def load_plugins(self, paths=None):
        pass
    monkeypatch.setattr(Editor, "load_plugins", load_plugins)
    assert main([]) == 1


def test_main_with_window(monkeypatch):
    def load_plugins(self, paths=None):
        self.window = object()
    monkeypatch.setattr(Editor, "load_plugins", load_plugins)
    assert main([]) == 0

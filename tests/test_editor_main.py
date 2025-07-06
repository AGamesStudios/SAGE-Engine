from sage_editor import main, Editor
from engine.utils import log as elog
import engine.core.engine as core_engine


def test_main_no_window(monkeypatch):
    monkeypatch.setattr(elog, "init_logger", lambda enable_crash_dumps=True: elog.logger)
    monkeypatch.setattr(core_engine, "_exception_handler", lambda *a: None)
    def load_plugins(self, paths=None):
        pass
    monkeypatch.setattr(Editor, "load_plugins", load_plugins)
    assert main([]) == 1


def test_main_with_window(monkeypatch):
    monkeypatch.setattr(elog, "init_logger", lambda enable_crash_dumps=True: elog.logger)
    monkeypatch.setattr(core_engine, "_exception_handler", lambda *a: None)
    def load_plugins(self, paths=None):
        self.window = object()
    monkeypatch.setattr(Editor, "load_plugins", load_plugins)
    assert main([]) == 0

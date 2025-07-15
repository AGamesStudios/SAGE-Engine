from sage_engine.editor import main, Editor
from sage_engine.utils import log as elog
import sage_engine.core.engine as core_engine


def test_main_no_window(monkeypatch):
    msgs = []
    monkeypatch.setattr(elog, "init_logger", lambda enable_crash_dumps=True: elog.logger)
    monkeypatch.setattr(core_engine, "_exception_handler", lambda *a: None)
    monkeypatch.setattr(elog.logger, "error", lambda msg: msgs.append(msg))
    def load_plugins(self, paths=None):
        pass
    monkeypatch.setattr(Editor, "load_plugins", load_plugins)
    assert main() == 1
    assert any("No editor window" in m for m in msgs)


def test_main_with_window(monkeypatch):
    msgs = []
    monkeypatch.setattr(elog, "init_logger", lambda enable_crash_dumps=True: elog.logger)
    monkeypatch.setattr(core_engine, "_exception_handler", lambda *a: None)
    monkeypatch.setattr(elog.logger, "info", lambda msg: msgs.append(msg))
    def load_plugins(self, paths=None):
        self.window = object()
    monkeypatch.setattr(Editor, "load_plugins", load_plugins)
    assert main() == 0
    assert any("SAGE Editor started" in m for m in msgs)

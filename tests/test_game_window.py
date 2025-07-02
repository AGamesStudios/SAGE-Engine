import types
import sys

qtwidgets = types.ModuleType('PyQt6.QtWidgets')
qtcore = types.ModuleType('PyQt6.QtCore')

class DummyTimer:
    def __init__(self, parent=None):
        self.stopped = False
        self._callback = None
    def setInterval(self, ms):
        pass
    def timeout(self):
        return types.SimpleNamespace(connect=lambda cb: setattr(self, '_callback', cb))
    timeout = property(timeout)
    def start(self):
        pass
    def stop(self):
        self.stopped = True

qtcore.QTimer = DummyTimer
qtcore.Qt = types.SimpleNamespace(WidgetAttribute=types.SimpleNamespace(WA_DeleteOnClose=0))
qtcore.pyqtSignal = lambda *a, **k: lambda *args, **kwargs: None
class DummyMainWindow:
    def __init__(self, *a, **k):
        pass
    def setWindowTitle(self, title):
        pass
    def setCentralWidget(self, w):
        pass
    def setAttribute(self, *a, **k):
        pass
    def resize(self, w, h):
        pass

class DummyWidget:
    def __init__(self, *a, **k):
        pass

class DummyLayout:
    def __init__(self, *a, **k):
        pass
    def setContentsMargins(self, *a):
        pass
    def addWidget(self, w):
        pass

qtwidgets.QMainWindow = DummyMainWindow
qtwidgets.QWidget = DummyWidget
qtwidgets.QVBoxLayout = DummyLayout

sys.modules.setdefault('PyQt6', types.ModuleType('PyQt6'))
sys.modules['PyQt6.QtWidgets'] = qtwidgets
sys.modules['PyQt6.QtCore'] = qtcore

from engine import game_window  # noqa: E402

class DummyEngine:
    def __init__(self):
        self.renderer = types.SimpleNamespace(title='T', width=1, height=1, widget=None)
        self.fps = 0
    def step(self):
        raise RuntimeError('fail')

class DummyLogger:
    def __init__(self):
        self.called = False
        self.msg = ''
    def exception(self, msg):
        self.called = True
        self.msg = msg

def test_step_logs_exception(monkeypatch):
    logger = DummyLogger()
    monkeypatch.setattr(game_window, 'logger', logger)
    win = game_window.GameWindow(DummyEngine())
    win._step()
    assert logger.called
    assert 'Engine step failed' in logger.msg
    assert win.timer.stopped

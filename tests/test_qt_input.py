import types
import sys

# Provide minimal PyQt6 stubs
qtcore = types.ModuleType('PyQt6.QtCore')
class QObject:
    def __init__(self):
        pass
    def installEventFilter(self, f):
        self.filter = f
    def removeEventFilter(self, f):
        self.filter = None
qtcore.QObject = QObject
qtcore.QEvent = types.SimpleNamespace(
    Type=types.SimpleNamespace(
        KeyPress=1,
        KeyRelease=2,
        MouseButtonPress=3,
        MouseButtonRelease=4,
    )
)

sys.modules.setdefault('PyQt6', types.ModuleType('PyQt6'))
sys.modules['PyQt6.QtCore'] = qtcore

from engine.inputs import get_input, INPUT_REGISTRY  # noqa: E402

INPUT_REGISTRY.clear()
cls = get_input('qt')


def test_qt_backend_registered():
    assert cls.__name__ == 'QtInput'


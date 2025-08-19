
from .bridge_base import ScriptBridge
import importlib

class CppBridge(ScriptBridge):
    lang = "cpp"
    def __init__(self, api=None):
        super().__init__(api)
        self.module = None

    def load(self, entry, **kw):
        try:
            self.module = importlib.import_module(entry)  # pybind11 or C-extension name
            return True
        except Exception as e:
            print("[CppBridge] import failed:", e)
            return False

    def call(self, fn, *args, **kwargs):
        if not self.module:
            return None
        f = getattr(self.module, fn, None)
        if callable(f):
            return f(*args, **kwargs)
        return None


import importlib, types, traceback
from .bridge_base import ScriptBridge

class PythonBridge(ScriptBridge):
    lang = "python"
    def __init__(self, api=None, module_search=None):
        super().__init__(api)
        self.module_search = module_search or []
        self.module = None

    def load(self, entry, **kw):
        try:
            self.module = importlib.import_module(entry)
            if hasattr(self.module, "init"):
                self.module.init(self.api)
            return True
        except Exception as e:
            print("[PythonBridge] load failed:", e)
            traceback.print_exc()
            return False

    def call(self, fn, *args, **kwargs):
        if not self.module:
            return None
        f = getattr(self.module, fn, None)
        if callable(f):
            return f(*args, **kwargs)
        return None

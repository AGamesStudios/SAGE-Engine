
from .bridge_base import ScriptBridge
try:
    import clr  # pythonnet
except Exception:
    clr = None

class CSharpBridge(ScriptBridge):
    lang = "csharp"
    def __init__(self, api=None):
        super().__init__(api)
        self.assembly = None

    def load(self, entry, **kw):
        if clr is None:
            print("[CSharpBridge] pythonnet not installed; pip install pythonnet")
            return False
        try:
            clr.AddReference(entry)
            import System
            self.System = System
            return True
        except Exception as e:
            print("[CSharpBridge] AddReference failed:", e)
            return False

    def call(self, fn, *args, **kwargs):
        # expects a static class Program with method fn
        try:
            from Program import Program
            if hasattr(Program, fn):
                return getattr(Program, fn)(*args, **kwargs)
        except Exception:
            pass
        return None

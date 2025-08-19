
from .bridge_base import ScriptBridge
try:
    from lupa import LuaRuntime
except Exception:
    LuaRuntime = None

class LuaBridge(ScriptBridge):
    lang = "lua"
    def __init__(self, api=None):
        super().__init__(api)
        self.lua = None
        self.globals = None

    def load(self, entry, code=None, **kw):
        if LuaRuntime is None:
            print("[LuaBridge] lupa not installed; pip install lupa (requires LuaJIT build)")
            return False
        self.lua = LuaRuntime(unpack_returned_tuples=True)
        self.globals = self.lua.globals()
        # expose minimal API
        self.globals["api"] = self.api
        if code:
            self.lua.execute(code)
        else:
            with open(entry, "r", encoding="utf-8") as f:
                self.lua.execute(f.read())
        return True

    def call(self, fn, *args, **kwargs):
        if not self.globals:
            return None
        f = self.globals.get(fn)
        if f:
            return f(*args, **kwargs)
        return None

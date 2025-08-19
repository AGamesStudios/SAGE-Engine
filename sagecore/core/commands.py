
class CommandRegistry:
    _inst = None
    def __init__(self):
        self._cmds = {}; self._default_engine = None; self._unknown_log = []
    @classmethod
    def get(cls):
        if not cls._inst: cls._inst = cls()
        return cls._inst
    def set_default_engine(self, eng): self._default_engine = eng
    def register(self, name, fn, safe=True, origin='core', trust='trusted'):
        self._cmds[name] = {'fn':fn,'safe':bool(safe),'origin':origin,'trust':trust}
    def execute(self, name, *args, **kwargs):
        rec = self._cmds.get(name)
        if not rec:
            self._unknown_log.append(name); return f"error: unknown command '{name}'"
        fn = rec['fn']
        try: return fn(self._default_engine, *args, **kwargs)
        except TypeError: return fn(*args, **kwargs)
    def list(self): return sorted(self._cmds.keys())
    def unknown_log(self): return list(self._unknown_log)


class EventBus:
    _inst = None
    def __init__(self): self._subs = {}
    @classmethod
    def get(cls):
        if not cls._inst: cls._inst = cls()
        return cls._inst
    def on(self, name, fn): self._subs.setdefault(name, []).append(fn)
    def off(self, name, fn):
        if name in self._subs and fn in self._subs[name]: self._subs[name].remove(fn)
    def emit(self, name, **payload):
        for fn in list(self._subs.get(name, [])):
            try: fn(**payload)
            except Exception: pass

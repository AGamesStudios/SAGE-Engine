
class EventsBus:
    def __init__(self):
        self._handlers = {}
    def on(self, evt, fn):
        self._handlers.setdefault(evt, []).append(fn)
    def emit(self, evt, *a, **kw):
        for fn in self._handlers.get(evt, []):
            try: fn(*a, **kw)
            except Exception as e: print("[events]", evt, e)

class PluginAPI:
    def __init__(self, engine=None):
        self._services = {}
        self._systems = []
        self._commands = {}
        self._events = EventsBus()
        self._engine = engine
    # services
    def provide_service(self, name, call):
        self._services.setdefault(name, []).append(call)
    def get_service(self, name):
        arr = self._services.get(name) or []
        return arr[0] if arr else None
    def get_services(self, name):
        return list(self._services.get(name) or [])
    # systems
    def register_system(self, name, fn, order=100):
        self._systems.append((order, name, fn))
        self._systems.sort(key=lambda x: x[0])
    def run_systems(self, engine, dt):
        for _, name, fn in self._systems:
            fn(engine, dt)
    # commands
    def register_command(self, name, fn, safe=True):
        self._commands[name] = fn
    def invoke_command(self, name, **kw):
        fn = self._commands.get(name)
        if fn: return fn(**kw)
    # hooks and events
    def events(self):
        return self._events
    def get_engine(self):
        return self._engine


class BehaviorSystem:
    _inst = None
    def __init__(self): self._map = {}
    @classmethod
    def get(cls):
        if not cls._inst: cls._inst = cls()
        return cls._inst
    def register_behavior(self, role_name, fn): self._map.setdefault(role_name, []).append(fn)
    def update_all(self, objects, dt, engine):
        for o in list(objects or []):
            role = getattr(o, 'role', None) or (o.get('role') if isinstance(o, dict) else None)
            for fn in self._map.get(role, []):
                try: fn(o, dt, engine)
                except Exception: pass

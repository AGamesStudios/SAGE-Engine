from collections import OrderedDict

class Cache:
    """Simple LRU cache used by the engine."""

    def __init__(self, max_items: int = 128):
        self.max_items = max_items
        self._store: OrderedDict[str, bytes] = OrderedDict()

    def get(self, key: str):
        value = self._store.get(key)
        if value is not None:
            self._store.move_to_end(key)
        return value

    def set(self, key: str, value: bytes) -> None:
        if key in self._store:
            self._store.move_to_end(key)
        self._store[key] = value
        while len(self._store) > self.max_items:
            self._store.popitem(last=False)

    def clear(self) -> None:
        self._store.clear()


SAGE_CACHE = Cache()

__all__ = ["Cache", "SAGE_CACHE"]

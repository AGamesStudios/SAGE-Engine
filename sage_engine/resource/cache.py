_cache = {}


def get(key: str):
    return _cache.get(key)


def set(key: str, data: bytes):
    _cache[key] = data


def clear():
    _cache.clear()

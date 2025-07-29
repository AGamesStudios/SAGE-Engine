_cache: dict[str, object] = {}


def get(key: str):
    return _cache.get(key)


def set(key: str, value: object):
    _cache[key] = value


def clear():
    _cache.clear()

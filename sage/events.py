from typing import Any, Callable

class EventSlot:
    __slots__ = ['handlers']
    def __init__(self) -> None:
        self.handlers: list[tuple[Callable[[Any], None], Any]] = []

_events: dict[str, EventSlot] = {}


def on(event: str, handler, *, owner=None) -> None:
    if not callable(handler):
        raise TypeError('handler must be callable')
    slot = _events.setdefault(event, EventSlot())
    slot.handlers.append((handler, owner))


def once(event: str, handler, *, owner=None) -> None:
    def wrapper(data):
        off(event, wrapper)
        handler(data)
    on(event, wrapper, owner=owner)


def off(event: str, handler) -> None:
    slot = _events.get(event)
    if not slot:
        return
    slot.handlers = [h for h in slot.handlers if h[0] is not handler]
    if not slot.handlers:
        _events.pop(event, None)


def emit(event: str, data=None) -> int:
    slot = _events.get(event)
    if not slot:
        return 0
    for func, _ in list(slot.handlers):
        func(data)
    return len(slot.handlers)


def cleanup_events() -> None:
    for event, slot in list(_events.items()):
        slot.handlers = [h for h in slot.handlers if not getattr(h[1], 'remove', False)]
        if not slot.handlers:
            _events.pop(event, None)


def register_events(obj) -> None:
    for key, value in obj.params.items():
        if key.startswith('on_'):
            event = key[3:]
            if isinstance(value, str):
                code = value.lstrip('@')
                def handler(data=None, code=code, obj=obj):
                    local_vars = {'obj': obj, 'data': data}
                    exec(code, local_vars)
                on(event, handler, owner=obj)
            elif callable(value):
                on(event, value, owner=obj)

__all__ = ['on', 'once', 'off', 'emit', 'cleanup_events', 'register_events']


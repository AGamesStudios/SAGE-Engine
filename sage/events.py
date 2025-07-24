from typing import Any, Callable
import asyncio
import inspect

class EventSlot:
    __slots__ = ['handlers']
    def __init__(self) -> None:
        self.handlers: list[tuple[Callable[[Any], None], Any]] = []

_events: dict[str, EventSlot] = {}
_filters: dict[str, list[Callable[[Any], Any]]] = {}


def on(event: str, handler, *, owner=None) -> None:
    if not callable(handler):
        raise TypeError('handler must be callable')
    slot = _events.setdefault(event, EventSlot())
    slot.handlers.append((handler, owner))


def async_on(event: str, handler, *, owner=None) -> None:
    """Register an async event handler."""
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
    for f in _filters.get(event, []):
        data = f(data)
    for func, _ in list(slot.handlers):
        if inspect.iscoroutinefunction(func):
            asyncio.create_task(func(data))
        else:
            func(data)
    return len(slot.handlers)


def add_filter(event: str, func: Callable[[Any], Any]) -> None:
    _filters.setdefault(event, []).append(func)


def remove_filter(event: str, func: Callable[[Any], Any]) -> None:
    lst = _filters.get(event)
    if lst and func in lst:
        lst.remove(func)
    if lst and not lst:
        _filters.pop(event, None)


def clear_filters() -> None:
    _filters.clear()


async def emit_async(event: str, data=None) -> int:
    slot = _events.get(event)
    if not slot:
        return 0
    for f in _filters.get(event, []):
        data = await f(data) if inspect.iscoroutinefunction(f) else f(data)
    for func, _ in list(slot.handlers):
        if inspect.iscoroutinefunction(func):
            await func(data)
        else:
            func(data)
    return len(slot.handlers)


def cleanup_events() -> None:
    for event, slot in list(_events.items()):
        slot.handlers = [h for h in slot.handlers if not getattr(h[1], 'remove', False)]
        if not slot.handlers:
            _events.pop(event, None)


def get_event_handlers() -> dict[str, list[Callable]]:
    """Return current event -> handlers mapping (for debugging)."""
    return {evt: [h[0] for h in slot.handlers] for evt, slot in _events.items()}


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

__all__ = [
    'on',
    'async_on',
    'once',
    'off',
    'emit',
    'emit_async',
    'add_filter',
    'remove_filter',
    'clear_filters',
    'cleanup_events',
    'register_events',
    'get_event_handlers',
]


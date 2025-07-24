from typing import Any, Callable
import asyncio
import inspect
import logging

class EventSlot:
    __slots__ = ['handlers']
    def __init__(self) -> None:
        self.handlers: list[tuple[Callable[[Any], None], Any]] = []

_events: dict[str, EventSlot] = {}
_filters: dict[str, list[Callable[[Any], Any]]] = {}

_LOGGER = logging.getLogger(__name__)


def on(event: str, handler, *, owner=None) -> None:
    if not callable(handler):
        raise TypeError('handler must be callable')
    try:
        sig = inspect.signature(handler)
        params = list(sig.parameters.values())
        positional = [p for p in params if p.kind in (inspect.Parameter.POSITIONAL_ONLY, inspect.Parameter.POSITIONAL_OR_KEYWORD, inspect.Parameter.KEYWORD_ONLY)]
        param_count = len(positional)
        if param_count == 0 and any(p.kind in (inspect.Parameter.VAR_POSITIONAL, inspect.Parameter.VAR_KEYWORD) for p in params):
            param_count = 1
    except (ValueError, TypeError):
        param_count = 1
    if param_count > 1:
        raise ValueError(f"Handler '{getattr(handler, '__name__', repr(handler))}' must accept 0 or 1 parameters")
    slot = _events.setdefault(event, EventSlot())
    slot.handlers.append((handler, owner))


def async_on(event: str, handler, *, owner=None) -> None:
    """Register an async event handler."""
    if not callable(handler):
        raise TypeError('handler must be callable')
    try:
        sig = inspect.signature(handler)
        params = list(sig.parameters.values())
        positional = [p for p in params if p.kind in (inspect.Parameter.POSITIONAL_ONLY, inspect.Parameter.POSITIONAL_OR_KEYWORD, inspect.Parameter.KEYWORD_ONLY)]
        param_count = len(positional)
        if param_count == 0 and any(p.kind in (inspect.Parameter.VAR_POSITIONAL, inspect.Parameter.VAR_KEYWORD) for p in params):
            param_count = 1
    except (ValueError, TypeError):
        param_count = 1
    if param_count > 1:
        raise ValueError(f"Handler '{getattr(handler, '__name__', repr(handler))}' must accept 0 or 1 parameters")
    slot = _events.setdefault(event, EventSlot())
    slot.handlers.append((handler, owner))


def once(event: str, handler, *, owner=None) -> None:
    try:
        sig = inspect.signature(handler)
        params = list(sig.parameters.values())
        positional = [p for p in params if p.kind in (inspect.Parameter.POSITIONAL_ONLY, inspect.Parameter.POSITIONAL_OR_KEYWORD, inspect.Parameter.KEYWORD_ONLY)]
        param_count = len(positional)
        if param_count == 0 and any(p.kind in (inspect.Parameter.VAR_POSITIONAL, inspect.Parameter.VAR_KEYWORD) for p in params):
            param_count = 1
    except (ValueError, TypeError):
        param_count = 1
    if param_count > 1:
        raise ValueError(f"Handler '{getattr(handler, '__name__', repr(handler))}' must accept 0 or 1 parameters")

    def wrapper(data=None):
        off(event, wrapper)
        if param_count == 0:
            handler()
        else:
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
    _LOGGER.info(f"[event_emit] '{event}' (data={data}) dispatched to {len(slot.handlers)} handler(s)")
    for func, _ in list(slot.handlers):
        try:
            params = inspect.signature(func).parameters
            if len(params) == 0:
                if inspect.iscoroutinefunction(func):
                    asyncio.create_task(func())
                else:
                    func()
            else:
                if inspect.iscoroutinefunction(func):
                    asyncio.create_task(func(data))
                else:
                    func(data)
        except Exception as e:
            _LOGGER.error(f"[event_emit] handler '{func.__name__}' failed: {e}")
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
    _LOGGER.info(f"[event_emit] '{event}' (data={data}) dispatched to {len(slot.handlers)} handler(s)")
    for func, _ in list(slot.handlers):
        try:
            params = inspect.signature(func).parameters
            if inspect.iscoroutinefunction(func):
                if len(params) == 0:
                    await func()
                else:
                    await func(data)
            else:
                if len(params) == 0:
                    func()
                else:
                    func(data)
        except Exception as e:
            _LOGGER.error(f"[event_emit] handler '{func.__name__}' failed: {e}")
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


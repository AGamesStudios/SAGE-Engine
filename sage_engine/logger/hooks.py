from __future__ import annotations

import asyncio
import signal
import sys
import threading
from types import FrameType

from .crash import log_crash

_prev_sys_hook = None
_prev_thread_hook = None
_prev_asyncio_hook = None
_prev_signal_hooks = {}


def _handle_exception(exc_type, exc, tb):
    log_crash(exc_type, exc, tb)
    if _prev_sys_hook:
        _prev_sys_hook(exc_type, exc, tb)


def _handle_thread_exception(args: threading.ExceptHookArgs):
    log_crash(args.exc_type, args.exc_value, args.exc_traceback)
    if _prev_thread_hook:
        _prev_thread_hook(args)


def _handle_asyncio(loop: asyncio.AbstractEventLoop, context):
    exc = context.get("exception")
    if exc:
        log_crash(type(exc), exc, exc.__traceback__)
    if _prev_asyncio_hook:
        _prev_asyncio_hook(loop, context)


def _handle_signal(signum, frame: FrameType | None):
    code = f"SAGE_ERR_FATAL_SIGNAL_SIG{signal.Signals(signum).name}"
    if signum == signal.SIGINT:
        # Ctrl+C should report a clean interruption without attempting to
        # format the current frame as a traceback.
        log_crash(KeyboardInterrupt, KeyboardInterrupt(), None, code=code)
    else:
        # Other signals are logged as runtime errors; we avoid passing the
        # ``frame`` object to ``log_crash`` because it expects a traceback.
        log_crash(RuntimeError, RuntimeError(code), None, code=code)
    if signum in _prev_signal_hooks and _prev_signal_hooks[signum]:
        _prev_signal_hooks[signum](signum, frame)


def install() -> None:
    global _prev_sys_hook, _prev_thread_hook, _prev_asyncio_hook
    if _prev_sys_hook is not None:
        return
    _prev_sys_hook = sys.excepthook
    _prev_thread_hook = threading.excepthook
    _prev_asyncio_hook = asyncio.get_event_loop().get_exception_handler()

    sys.excepthook = _handle_exception
    threading.excepthook = _handle_thread_exception
    asyncio.get_event_loop().set_exception_handler(_handle_asyncio)

    for sig in (signal.SIGINT, signal.SIGTERM, getattr(signal, "SIGSEGV", None)):
        if sig is None:
            continue
        _prev_signal_hooks[sig] = signal.getsignal(sig)
        try:
            signal.signal(sig, _handle_signal)
        except (ValueError, OSError):
            pass

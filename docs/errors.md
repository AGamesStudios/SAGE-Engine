"""Error handling and crash logging."""

The ``logger`` module provides :func:`log_crash` which records
uncaught exceptions.  It writes a JSON report to ``logs/`` and prints
the stack to the console.  If an invalid traceback is supplied the
logger emits a warning and falls back to ``traceback.print_exc``
instead of raising another error.

To test crash handling call ``log_crash`` manually::

    try:
        1 / 0
    except Exception as exc:
        log_crash(type(exc), exc, exc.__traceback__)

Never let exceptions propagate silently; failing agents should report
their errors through ``log_crash`` and continue shutdown normally.


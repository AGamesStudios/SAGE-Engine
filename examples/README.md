# Running examples

Each example creates its own window and enters the engine loop::

    from sage_engine import core, window
    import time

    window.init("Example", 320, 200)
    core.core_boot()
    try:
        while not window.should_close():
            core.core_tick()
            time.sleep(1 / 60)
    finally:
        core.core_shutdown()

Always wrap the loop in ``try/finally`` so shutdown runs even if an
error occurs.


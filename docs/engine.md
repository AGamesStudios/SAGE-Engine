"""SAGE Engine bootstrap overview."""

The engine performs a simple phase cycle:

``boot → update → draw → flush → shutdown``

During `core.core_boot()` modules listed in `engine.sagecfg` are
imported and their boot callbacks executed.  A window must be created
before entering the main loop.  Examples typically call::

    window.init("Example", 640, 360)
    core.core_boot()
    try:
        while not window.should_close():
            core.core_tick()
            time.sleep(1 / 60)
    finally:
        core.core_shutdown()

Always guard the loop with ``try/finally`` so the engine shuts down
cleanly even if an error occurs.


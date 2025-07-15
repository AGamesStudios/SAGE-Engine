# Optimisation Tips

To keep large projects running smoothly consider the following options:

- **Headless mode** – Run the engine with the `NullRenderer` to skip all drawing
  when running automated tests or servers.
- **Keep sprite counts low** – Batch updates and reuse sprite data where
  possible to reduce per-frame work.
- **Use Numba or Cython** – Installing `numba` or compiling heavy modules with
  Cython can greatly accelerate math-heavy code.
- **Run on PyPy** – The engine is pure Python and works on PyPy 3.10+. PyPy's
  JIT compiler can improve performance for CPU bound projects without any
  code changes.
- **Tune sprite cache** – Each ``Engine`` manages its own sprite images. Set ``image_cache_limit`` in ``sage.toml`` or configure it via ``EngineSettings`` or ``set_sprite_cache`` to control how many sprites are cached per instance.
- **Adjust VSync/FPS** – Disable VSync (`--no-vsync`) or lower ``fps`` in
  ``EngineSettings`` on older machines to reduce rendering overhead.
- **Asynchronous events** – Large scenes can update events concurrently using `EventSystem.update_async(engine, scene, dt)` or `await EventSystem.update_asyncio(engine, scene, dt)`
  to reduce frame time. Use a thread-safe variable store when events modify shared data.
  When enabling ``asyncio_events`` the engine keeps a single event loop for updates.
  Enable concurrent updates globally with:

  ```python
  eng = Engine(scene=my_scene, async_events=True, event_workers=8)
  await eng.run_async()
  ```
  Extension hooks and object updates may be coroutines when using
  ``run_async``, allowing non-blocking logic.

  You can also profile groups of events by priority to identify bottlenecks:

  ```python
  for name in eng.events.get_group_names():
      eng.events.update_group(name, eng, eng.scene, 0)
      # measure time per group here
  ```
  Event priority can also be used to batch expensive actions:

  ```python
  eng.events.add_event(Event([...], [...], priority=10, groups=["physics"]))
  eng.events.update_group("physics", eng, eng.scene, dt)
  ```

`engine.utils.profiler.Profiler` can capture statistics for a section of code:

```python
from sage_engine.utils.profiler import Profiler
with Profiler("run.prof"):
    Engine(scene=my_scene).run()
```


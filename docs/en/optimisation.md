# Optimisation Tips

To keep large projects running smoothly consider the following options:

- **Headless mode** – Run the engine with the `NullRenderer` to skip all drawing
  when running automated tests or servers.
- **Keep sprite counts low** – Batch updates and reuse sprite data where
  possible to reduce per-frame work.
- **Use Numba or Cython** – Installing `numba` or compiling heavy modules with
  Cython can greatly accelerate math-heavy code.
- **Tune image cache** – Set `SAGE_IMAGE_CACHE_LIMIT` or pass
  ``image_cache_limit`` via `EngineSettings` to control how many sprites are
  kept in memory.
- **Asynchronous events** – Large scenes can update events concurrently using
  `EventSystem.update_async(engine, scene, dt)` to reduce frame time.


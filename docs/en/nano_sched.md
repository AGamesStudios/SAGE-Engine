# NanoSched

`run_sched()` executes a set of `PatcherTask` instances according to read/write masks. Dependencies are derived automatically and tasks run concurrently using a thread pool with up to eight workers.

# ChronoPatch Tree

The ChronoPatch Tree stores engine state in a single memory mapped file split
into ``SmartSlice`` blocks. Each slice is 1 KiB. Changes are recorded as LZMA
compressed patches so the state can be replayed or rolled back.

Use :class:`engine.chrono_patch.ChronoPatchTree` to manage the state. The
``snapshot`` method writes data to the mapped region and appends a patch entry to
``<path>.log``. Calling ``replay`` applies the patch log to restore the latest
state.

# MicroPython scripting

Feather Core embeds a minimal MicroPython runtime to allow small scripts to update object state while the engine is running.

Scripts can be registered via the FFI interface and executed on demand. The runtime is sandboxed: only the Python standard library is available and there is no filesystem access.

Example using `ctypes` to run a script:

```python
import ctypes
from pathlib import Path
lib = ctypes.CDLL(str(Path('rust/feather_core/target/release/libfeather_core.so')))
lib.mp_new.restype = ctypes.c_void_p
lib.mp_exec.argtypes = [ctypes.c_void_p, ctypes.c_char_p]
lib.mp_free.argtypes = [ctypes.c_void_p]

mp = lib.mp_new()
lib.mp_exec(mp, b"x = 1 + 1")
lib.mp_free(mp)
```

The script can modify data passed by the engine or interact with other FFI functions. This mechanism is used by the engine's patch system to apply dynamic behaviour without restarting the game.

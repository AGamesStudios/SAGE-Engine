import subprocess
import ctypes
from pathlib import Path

LIB_PATH = Path('rust/feather_core/target/release')


def build_lib():
    if not LIB_PATH.exists():
        subprocess.run(['cargo', 'build', '--release'], cwd='rust/feather_core', check=True)
    else:
        lib = LIB_PATH / 'libfeather_core.so'
        if not lib.exists():
            subprocess.run(['cargo', 'build', '--release'], cwd='rust/feather_core', check=True)
    return LIB_PATH / 'libfeather_core.so'


def test_run_script():
    lib = ctypes.CDLL(str(build_lib()))
    mp_new = lib.mp_new
    mp_new.restype = ctypes.c_void_p
    mp_exec = lib.mp_exec
    mp_exec.argtypes = [ctypes.c_void_p, ctypes.c_char_p]
    mp_exec.restype = ctypes.c_bool
    mp_free = lib.mp_free
    mp_free.argtypes = [ctypes.c_void_p]

    mp = mp_new()
    assert mp
    assert mp_exec(mp, b"x = 2 + 2")
    mp_free(mp)


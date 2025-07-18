import os
import subprocess
import shutil
import ctypes
import sys
from pathlib import Path

def build_lib() -> Path:
    cargo_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', 'rust', 'feather_core'))
    if not os.path.isdir(cargo_dir):
        raise NotADirectoryError(f"Не найдена папка: {cargo_dir}")
    cargo = shutil.which('cargo')
    if cargo is None:
        raise RuntimeError('cargo не найден в PATH')
    subprocess.run(['cargo', 'build', '--release'], cwd=cargo_dir, check=True)
    lib_name = 'libfeather_core.so'
    if os.name == 'nt':
        lib_name = 'feather_core.dll'
    elif sys.platform == 'darwin':
        lib_name = 'libfeather_core.dylib'
    return Path(cargo_dir) / 'target' / 'release' / lib_name


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


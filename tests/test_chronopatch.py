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


def test_basic_patch(tmp_path):
    lib_path = build_lib()
    lib = ctypes.CDLL(str(lib_path))
    cpt_new = lib.cpt_new
    cpt_new.restype = ctypes.c_void_p
    cpt_new.argtypes = [ctypes.c_char_p, ctypes.c_size_t]
    cpt_write = lib.cpt_write
    cpt_write.argtypes = [ctypes.c_void_p, ctypes.c_size_t, ctypes.c_void_p, ctypes.c_size_t]
    cpt_write.restype = ctypes.c_bool
    cpt_read = lib.cpt_read
    cpt_read.argtypes = [ctypes.c_void_p, ctypes.c_size_t, ctypes.c_void_p, ctypes.c_size_t]
    cpt_read.restype = ctypes.c_bool
    cpt_apply = lib.cpt_apply
    cpt_apply.argtypes = [ctypes.c_void_p]
    cpt_revert = lib.cpt_revert
    cpt_revert.argtypes = [ctypes.c_void_p]
    cpt_free = lib.cpt_free
    cpt_free.argtypes = [ctypes.c_void_p]

    file_path = tmp_path / 'tree.bin'
    tree = cpt_new(str(file_path).encode('utf-8'), 4096)
    assert tree

    data = b'hello'
    buf = (ctypes.c_ubyte * len(data)).from_buffer_copy(data)
    assert cpt_write(tree, 0, buf, len(data))

    out = (ctypes.c_ubyte * len(data))()
    assert cpt_read(tree, 0, out, len(data))
    assert bytes(out) == data

    data2 = b'world'
    buf2 = (ctypes.c_ubyte * len(data2)).from_buffer_copy(data2)
    assert cpt_write(tree, 0, buf2, len(data2))

    cpt_revert(tree)
    out2 = (ctypes.c_ubyte * len(data))()
    assert cpt_read(tree, 0, out2, len(data))
    assert bytes(out2) == data

    cpt_apply(tree)
    out3 = (ctypes.c_ubyte * len(data2))()
    assert cpt_read(tree, 0, out3, len(data2))
    assert bytes(out3) == data2

    cpt_free(tree)

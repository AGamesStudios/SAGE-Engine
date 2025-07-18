import ctypes

from tests.utils import build_lib


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


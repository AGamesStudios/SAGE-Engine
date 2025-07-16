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

CALLBACK = ctypes.CFUNCTYPE(None, ctypes.c_void_p)


def test_dag_order():
    lib = ctypes.CDLL(str(build_lib()))
    dag_new = lib.dag_new
    dag_new.restype = ctypes.c_void_p
    dag_add_task = lib.dag_add_task
    dag_add_task.argtypes = [ctypes.c_void_p, ctypes.c_size_t, CALLBACK, ctypes.c_void_p]
    dag_add_dependency = lib.dag_add_dependency
    dag_add_dependency.argtypes = [ctypes.c_void_p, ctypes.c_size_t, ctypes.c_size_t]
    dag_execute = lib.dag_execute
    dag_execute.argtypes = [ctypes.c_void_p]
    dag_execute.restype = ctypes.c_bool
    dag_free = lib.dag_free
    dag_free.argtypes = [ctypes.c_void_p]

    order = []
    callbacks = []

    def make_task(n):
        @CALLBACK
        def _task(_):
            order.append(n)
        callbacks.append(_task)
        return _task

    sched = dag_new()
    dag_add_task(sched, 1, make_task(1), None)
    dag_add_task(sched, 2, make_task(2), None)
    dag_add_task(sched, 3, make_task(3), None)
    dag_add_dependency(sched, 2, 1)
    dag_add_dependency(sched, 3, 2)
    assert dag_execute(sched)
    dag_free(sched)
    assert order == [1, 2, 3]


def test_dag_cycle():
    lib = ctypes.CDLL(str(build_lib()))
    dag_new = lib.dag_new
    dag_new.restype = ctypes.c_void_p
    dag_add_task = lib.dag_add_task
    dag_add_task.argtypes = [ctypes.c_void_p, ctypes.c_size_t, CALLBACK, ctypes.c_void_p]
    dag_add_dependency = lib.dag_add_dependency
    dag_add_dependency.argtypes = [ctypes.c_void_p, ctypes.c_size_t, ctypes.c_size_t]
    dag_execute = lib.dag_execute
    dag_execute.argtypes = [ctypes.c_void_p]
    dag_execute.restype = ctypes.c_bool
    dag_free = lib.dag_free
    dag_free.argtypes = [ctypes.c_void_p]

    sched = dag_new()
    keep = []

    @CALLBACK
    def noop(_):
        pass
    keep.append(noop)
    dag_add_task(sched, 1, noop, None)
    dag_add_task(sched, 2, noop, None)
    dag_add_dependency(sched, 1, 2)
    dag_add_dependency(sched, 2, 1)
    assert not dag_execute(sched)
    dag_free(sched)

import json
import os
import subprocess
import shutil
import ctypes
import sys
from pathlib import Path

from sage_engine import core_boot, core_reset

def build_lib() -> Path:
    cargo_dir = os.path.abspath(
        os.path.join(
            os.path.dirname(__file__),
            '..', '..',
            'rust', 'feather_core'
        )
    )
    if not os.path.isdir(cargo_dir):
        raise NotADirectoryError(f"Не найдена папка: {cargo_dir}")
    cargo = shutil.which('cargo')
    if cargo is None:
        raise RuntimeError('cargo не найден в PATH')
    print(f"[build_lib] Building Rust project in: {cargo_dir}")
    print(f"[build_lib] Using cargo at: {cargo}")
    try:
        subprocess.run(
            ['cargo', 'build', '--release', '--features', 'profiling'],
            cwd=cargo_dir,
            check=True,
        )
    except subprocess.CalledProcessError as e:
        print(f"[build_lib] cargo build failed with {e}")
        raise
    lib_name = 'libfeather_core.so'
    if os.name == 'nt':
        lib_name = 'feather_core.dll'
    elif sys.platform == 'darwin':
        lib_name = 'libfeather_core.dylib'
    return Path(cargo_dir) / 'target' / 'release' / lib_name

def main():
    # initialise Python side of the engine
    core_boot()
    lib_path = build_lib()
    lib = ctypes.CDLL(str(lib_path))

    lib.cpt_new.argtypes = [ctypes.c_char_p, ctypes.c_size_t]
    lib.cpt_new.restype = ctypes.c_void_p
    lib.cpt_write.argtypes = [ctypes.c_void_p, ctypes.c_size_t, ctypes.c_void_p, ctypes.c_size_t]
    lib.cpt_write.restype = ctypes.c_bool
    lib.cpt_read.argtypes = [ctypes.c_void_p, ctypes.c_size_t, ctypes.c_void_p, ctypes.c_size_t]
    lib.cpt_read.restype = ctypes.c_bool
    lib.cpt_free.argtypes = [ctypes.c_void_p]
    lib.mp_new.restype = ctypes.c_void_p
    lib.mp_free.argtypes = [ctypes.c_void_p]
    lib.mp_exec_json.argtypes = [ctypes.c_void_p, ctypes.c_char_p, ctypes.c_char_p]
    lib.mp_exec_json.restype = ctypes.c_void_p
    lib.mp_free_cstring.argtypes = [ctypes.c_void_p]
    lib.dag_new.restype = ctypes.c_void_p
    CALLBACK = ctypes.CFUNCTYPE(None, ctypes.c_void_p)
    lib.dag_add_task.argtypes = [ctypes.c_void_p, ctypes.c_size_t, CALLBACK, ctypes.c_void_p]
    lib.dag_execute.argtypes = [ctypes.c_void_p]
    lib.dag_execute.restype = ctypes.c_bool
    lib.dag_free.argtypes = [ctypes.c_void_p]

    tree = lib.cpt_new(b'demo_tree.bin', 1024)
    mp = lib.mp_new()

    state = {"pos": [0.0, 0.0], "velocity": [1.0, 0.0]}
    data = json.dumps(state).encode()
    buf = (ctypes.c_ubyte * len(data)).from_buffer_copy(data)
    lib.cpt_write(tree, 0, buf, len(data))

    # apply velocity patch
    state["velocity"] = [0.5, 0.5]
    data = json.dumps(state).encode()
    buf = (ctypes.c_ubyte * len(data)).from_buffer_copy(data)
    lib.cpt_write(tree, 0, buf, len(data))

    script = b"def update(state):\n    state['pos'][0] += state['velocity'][0]\n    state['pos'][1] += state['velocity'][1]\nupdate(state)"

    def run_script(_):
        nonlocal state, data
        data = json.dumps(state).encode()
        res = lib.mp_exec_json(mp, script, data)
        out = ctypes.cast(res, ctypes.c_char_p).value.decode()
        lib.mp_free_cstring(res)
        state = json.loads(out)
        data = json.dumps(state).encode()
        buf = (ctypes.c_ubyte * len(data)).from_buffer_copy(data)
        lib.cpt_write(tree, 0, buf, len(data))

    cb = CALLBACK(run_script)

    dag = lib.dag_new()
    lib.dag_add_task(dag, 1, cb, None)
    lib.dag_execute(dag)
    lib.dag_free(dag)

    out = (ctypes.c_ubyte * len(data))()
    lib.cpt_read(tree, 0, out, len(data))
    print("final state", json.loads(bytes(out).decode()))

    lib.mp_free(mp)
    lib.cpt_free(tree)

    # allow hot reinitialisation in development
    core_reset()

if __name__ == "__main__":
    main()

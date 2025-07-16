import json
import subprocess
import ctypes
from pathlib import Path

def build_lib():
    lib_path = Path('rust/feather_core/target/release/libfeather_core.so')
    subprocess.run(
        ['cargo', 'build', '--release', '--features', 'profiling'],
        cwd='rust/feather_core',
        check=True,
    )
    return lib_path

def main():
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

if __name__ == "__main__":
    main()

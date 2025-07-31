import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SDK = ROOT / 'gf_sdk'

def test_build_static():
    make = subprocess.run(['make', 'libgf_sdk.so'], cwd=SDK, capture_output=True, text=True)
    assert make.returncode == 0, make.stderr
    assert (SDK / 'libgf_sdk.so').exists()

def test_errors_api():
    from ctypes import CDLL, c_int, c_char_p
    lib = CDLL(str(SDK/'libgf_sdk.so'))
    lib.gf_strerror.restype = c_char_p
    lib.gf_error_name.restype = c_char_p
    code = c_int(0x01010001)  # CORE/MATH/EINVAL
    msg = lib.gf_strerror(code)
    name = lib.gf_error_name(code)
    assert msg.decode().startswith('Invalid')
    assert name.decode() == 'CORE/MATH/EINVAL'

def test_last_error():
    from ctypes import CDLL, c_int, c_char_p, c_void_p, Structure, c_uint32, c_uint16, c_float, c_int32, c_char, POINTER, byref
    lib = CDLL(str(SDK/'libgf_sdk.so'))
    class gf_cfg(Structure):
        _fields_=[('target_fps', c_uint32), ('window_ms', c_uint32), ('drop_fps', c_uint16),
                  ('ema_alpha', c_float), ('ring_capacity', c_uint32)]
    class gf_error_info(Structure):
        _fields_=[('code', c_int32), ('detail', c_uint32), ('line', c_uint32), ('where', c_char*24)]
    ctx = c_void_p()
    cfg = gf_cfg(60,5000,30,c_float(0.2),300)
    lib.gf_init.argtypes=[POINTER(c_void_p), POINTER(gf_cfg)]
    assert lib.gf_init(byref(ctx), byref(cfg)) == 0
    lib.gf_metrics.argtypes=[c_void_p, c_void_p]
    rc = lib.gf_metrics(ctx, c_void_p(None))
    assert rc != 0
    info = gf_error_info()
    lib.gf_last_error.argtypes=[c_void_p, POINTER(gf_error_info)]
    lib.gf_last_error.restype = c_int
    lib.gf_error_name.restype = c_char_p
    assert lib.gf_last_error(ctx, byref(info)) == 0
    assert info.code == rc
    name = lib.gf_error_name(info.code).decode()
    assert name == 'CORE/MATH/EINVAL'

import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SDK = ROOT / 'gf_sdk'

def test_build_static():
    make = subprocess.run(['make', 'libgf_sdk.so'], cwd=SDK, capture_output=True, text=True)
    assert make.returncode == 0, make.stderr
    assert (SDK / 'libgf_sdk.so').exists()

def test_strerror():
    from ctypes import CDLL, c_int, c_char_p
    lib = CDLL(str(SDK/'libgf_sdk.so'))
    lib.gf_strerror.restype = c_char_p
    msg = lib.gf_strerror(c_int(1))
    assert msg.decode() == 'invalid argument'

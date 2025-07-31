import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SDK = ROOT / 'gf_sdk'

def test_build_static():
    make = subprocess.run(['make'], cwd=SDK, capture_output=True, text=True)
    assert make.returncode == 0, make.stderr
    assert (SDK / 'libgf_sdk.a').exists()

import os
import subprocess
import shutil
import sys
from pathlib import Path


def build_lib() -> Path:
    cargo_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', 'rust', 'feather_core'))
    if not os.path.isdir(cargo_dir):
        raise NotADirectoryError(f"Не найдена папка: {cargo_dir}")
    cargo = shutil.which('cargo')
    if cargo is None:
        raise RuntimeError('cargo не найден в PATH')
    subprocess.run([
        'cargo',
        'build',
        '--release',
        '--features',
        'profiling',
    ], cwd=cargo_dir, check=True)
    lib_name = 'libfeather_core.so'
    if os.name == 'nt':
        lib_name = 'feather_core.dll'
    elif sys.platform == 'darwin':
        lib_name = 'libfeather_core.dylib'
    return Path(cargo_dir) / 'target' / 'release' / lib_name

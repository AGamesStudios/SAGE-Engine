import subprocess
import sys


def test_gui_list():
    out = subprocess.check_output([
        sys.executable,
        "-m",
        "sage_cli",
        "run",
        "examples/hello_sprite/main.py",
        "--gui",
        "list",
    ], text=True, env={"PYTHONPATH": "src"})
    assert "tk" in out

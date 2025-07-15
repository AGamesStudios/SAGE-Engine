import subprocess
import sys

def test_bootstrap_import():
    subprocess.check_call([sys.executable, '-c', 'import sage_engine'])

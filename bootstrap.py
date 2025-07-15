import subprocess
import sys

subprocess.check_call([sys.executable, "-m", "pip", "install", "-e", ".[dev]"])
print("SAGE Engine ready")

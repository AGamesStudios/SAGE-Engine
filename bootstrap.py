import subprocess
import sys
import importlib.metadata as md

subprocess.check_call([sys.executable, "-m", "pip", "install", "-e", ".[dev]"])
print("SAGE Engine ready")
print("Installed:", md.version("sage-engine"))

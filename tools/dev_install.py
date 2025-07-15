"""Install the package in editable mode with dev extras."""
import subprocess
import sys

subprocess.check_call([sys.executable, "-m", "pip", "install", "-e", ".[dev]"])

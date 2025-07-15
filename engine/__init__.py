"""Temporary compatibility alias. Remove after version 1.1."""
from importlib import import_module
import sys

sys.modules[__name__] = import_module('sage_engine')

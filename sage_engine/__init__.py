"""Local-import convenience for tests.

TODO: remove after version 1.1.
"""
from importlib import import_module
import sys

sys.modules[__name__] = import_module('src.sage_engine')

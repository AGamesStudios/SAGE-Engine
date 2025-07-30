import importlib
from sage_engine import core

modules = [
    "window",
    "input",
    "blueprint",
    "resource",
    "objects",
]

def test_exposed_interfaces():
    for name in modules:
        importlib.import_module(f"sage_engine.{name}")
        assert core.get(name) is not None, f"{name} not exposed"

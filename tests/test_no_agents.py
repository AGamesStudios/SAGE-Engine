import os
import importlib
import pkgutil


def test_no_agent_terms():
    for root, dirs, files in os.walk('sage_engine'):
        for d in dirs:
            assert 'agent' not in d.lower()
        for f in files:
            if f.endswith('.py'):
                path = os.path.join(root, f)
                with open(path, 'r', encoding='utf-8') as fh:
                    data = fh.read().lower()
                    assert 'agent' not in data


def test_systems_importable():
    pkg = importlib.import_module('sage_engine.objects.groups')
    assert hasattr(pkg, 'system')
    mods = [m.name for m in pkgutil.iter_modules(pkg.__path__)]
    assert 'system' in mods

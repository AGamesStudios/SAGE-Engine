import types
import sage_engine.core as core
import pytest


def test_register_duplicate(monkeypatch):
    factories = {}
    systems = {}
    monkeypatch.setattr(core, 'SUBSYSTEM_FACTORIES', factories)
    monkeypatch.setattr(core, 'SUBSYSTEMS', systems)
    core.register_subsystem('demo', lambda: types.ModuleType('demo'))
    with pytest.raises(ValueError):
        core.register_subsystem('demo', lambda: types.ModuleType('demo'))


def test_disabled_subsystem(monkeypatch):
    factories = {}
    systems = {}
    boot_order = ['demo']
    called = []
    demo_mod = types.ModuleType('demo')
    demo_mod.boot = lambda: called.append('boot')
    factories['demo'] = lambda: demo_mod
    monkeypatch.setattr(core, 'SUBSYSTEM_FACTORIES', factories)
    monkeypatch.setattr(core, 'SUBSYSTEMS', systems)
    monkeypatch.setattr(core, 'BOOT_SEQUENCE', boot_order)
    monkeypatch.setattr(core, '_booted', False)
    monkeypatch.setattr(core, '_profile', None)
    monkeypatch.setattr(core, 'load_config', lambda: {
        'disabled_subsystems': ['demo'],
        'plugins': []
    })
    profile = core.core_boot()
    assert not called
    assert profile.entries

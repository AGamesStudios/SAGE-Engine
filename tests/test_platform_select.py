import sage_engine.platform as platform_mod


def test_force_platform(monkeypatch):
    monkeypatch.setattr(platform_mod, 'load_platform_config', lambda: {'force': 'linux'})
    assert platform_mod.get_platform() == 'linux'


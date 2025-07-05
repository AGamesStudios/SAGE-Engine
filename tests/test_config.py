import types
import engine.plugins


def test_plugin_config(tmp_path, monkeypatch):
    cfg = tmp_path / "sage.toml"
    plugdir = tmp_path / "plugins"
    plugdir.mkdir()
    (plugdir / "c.py").write_text("def init_engine(e): e.hit=True")
    cfg.write_text(f"[plugins]\ndir='{plugdir}'\n")
    monkeypatch.setenv("SAGE_CONFIG", str(cfg))
    import engine.utils.config as cfgmod
    cfgmod._config_cache = None
    obj = types.SimpleNamespace()
    engine.plugins.PluginManager('engine').load(obj)
    assert getattr(obj, 'hit', False)

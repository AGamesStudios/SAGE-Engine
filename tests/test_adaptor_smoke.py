import importlib
import pytest

ADAPTORS = [
    "sage_engine.adaptors.render",
    "sage_engine.adaptors.audio",
    "sage_engine.adaptors.network",
    "sage_engine.adaptors.gui",
    "sage_engine.adaptors.opengl",
]

@pytest.mark.adaptor
@pytest.mark.parametrize("mod_name", ADAPTORS)
def test_adaptor_smoke(mod_name):
    mod = importlib.import_module(mod_name)
    assert callable(mod.register)
    mod.register()
    caps = mod.get_capabilities()
    assert isinstance(caps, list)
    if mod_name == "sage_engine.adaptors.opengl":
        assert "render_opengl" in caps

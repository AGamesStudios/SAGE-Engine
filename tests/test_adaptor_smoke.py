import importlib
import pytest

ADAPTORS = [
    "sage_adaptors.render",
    "sage_adaptors.audio",
    "sage_adaptors.network",
    "sage_adaptors.gui",
    "sage_adaptors.opengl",
]

@pytest.mark.adaptor
@pytest.mark.parametrize("mod_name", ADAPTORS)
def test_adaptor_smoke(mod_name):
    mod = importlib.import_module(mod_name)
    assert callable(mod.register)
    mod.register()
    caps = mod.get_capabilities()
    assert isinstance(caps, list)
    if mod_name == "sage_adaptors.opengl":
        assert "render_opengl" in caps

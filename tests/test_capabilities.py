from sage_engine.capabilities import (
    caps_from_flags,
    missing_caps_from_flags,
    FEATURES,
    check_scene_caps,
)
from sage_engine.extras import lua


def test_caps_mapping():
    assert "volumetric-fx" in FEATURES
    flags = (1 << FEATURES.index("volumetric-fx"))
    assert caps_from_flags(flags) == ["volumetric-fx"]
    assert missing_caps_from_flags(flags) == []


def test_vm_lua_missing(monkeypatch):
    monkeypatch.setattr(lua, "AVAILABLE", False)

    class Dummy:
        metadata = {"caps": ["vm_lua"]}

    assert "vm_lua" in check_scene_caps(Dummy())


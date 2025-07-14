from engine.capabilities import caps_from_flags, missing_caps_from_flags, FEATURES


def test_caps_mapping():
    assert "volumetric-fx" in FEATURES
    flags = (1 << FEATURES.index("volumetric-fx"))
    assert caps_from_flags(flags) == ["volumetric-fx"]
    assert missing_caps_from_flags(flags) == []


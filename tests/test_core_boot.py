from sage_engine import core_boot, core_reset


def test_core_boot_and_reset():
    profile = core_boot()
    assert profile.entries
    # ensure reset does not raise and returns a new profile
    profile2 = core_reset()
    assert profile2.entries

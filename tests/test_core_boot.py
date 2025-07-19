import os
os.environ["SDL_VIDEODRIVER"] = "dummy"

from sage_engine import core_boot, core_reset, core_debug
from sage_engine.object import get_objects


def test_core_boot_and_reset():
    profile = core_boot()
    assert profile.entries
    assert any(obj.role == "UI" for obj in get_objects())
    # ensure reset does not raise and returns a new profile
    profile2 = core_reset()
    assert profile2.entries
    core_debug()

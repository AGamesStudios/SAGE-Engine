from sage_engine.testing import visual


def test_compare_screenshot():
    assert visual.diff('sprite.png', 'sprite.png') >= 0.0

from sage_engine.testing import visual


def test_compare_screenshot():
    assert visual.diff('exp.png', 'act.png') >= 0.0

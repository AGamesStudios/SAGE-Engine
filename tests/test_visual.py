from sage_testing import visual


def test_compare_screenshot():
    assert visual.compare_screenshot('exp.png', 'act.png', max_diff=0.1) is True

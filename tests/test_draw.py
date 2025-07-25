from sage_engine.draw import boot, reset, draw_line, draw_rect, draw_circle, get_calls


def test_draw_calls():
    boot()
    draw_line((0, 0), (1, 1))
    draw_rect((0, 0, 2, 2))
    draw_circle((1, 1), 1)
    calls = get_calls()
    assert ("line", (0, 0), (1, 1), (255, 255, 255), 1) in calls
    assert ("rect", (0, 0, 2, 2), (255, 255, 255), 1) in calls
    assert ("circle", (1, 1), 1, (255, 255, 255), 1) in calls
    reset()

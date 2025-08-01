def test_scroll_draw_restores_position():
    from sage_engine.gui import widgets
    sv = widgets.ScrollView(width=100, height=100)
    child = widgets.Button(width=10, height=10)
    child.y = 50
    sv.add_child(child)
    sv.scroll_y = 25
    sv.draw()
    assert child.y == 50

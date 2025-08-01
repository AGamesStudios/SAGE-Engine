from sage_engine.gui import widgets, layout


def test_hbox_layout_auto_size():
    container = widgets.Button()
    child1 = widgets.Button(width=5, height=5)
    child2 = widgets.Button(width=5, height=5)
    container.add_child(child1)
    container.add_child(child2)
    layout.HBoxLayout(spacing=1, auto_size=True).apply(container)
    assert container.width == child1.width + child2.width + 1
    assert child2.x == container.x + child1.width + 1


def test_vbox_layout_padding():
    container = widgets.Button()
    child = widgets.Button(width=4, height=3)
    container.add_child(child)
    layout.VBoxLayout(padding=2).apply(container)
    assert child.y == container.y + 2


def test_grid_layout_auto_size():
    cont = widgets.Button()
    for _ in range(4):
        cont.add_child(widgets.Button(width=2, height=1))
    gl = layout.GridLayout(cols=2, spacing=1, auto_size=True)
    gl.apply(cont)
    assert cont.height > 0 and cont.width > 0

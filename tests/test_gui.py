def test_gui_basic():
    from sage_engine.gui import manager, widgets

    btn = widgets.Button(width=10, height=10)
    lbl = widgets.Label(width=10, height=10)
    called = []
    btn.on_click.connect(lambda: called.append(True))

    manager.root.add_child(btn)
    manager.root.add_child(lbl)

    manager.dispatch_click(5, 5)
    assert called

def test_focus_event():
    from sage_engine.gui import manager, widgets

    w = widgets.Button()
    events = []
    w.on_focus.connect(lambda f: events.append(f))
    manager.set_focus(w)
    assert events == [True]


def test_hover_event():
    from sage_engine.gui import widgets

    w = widgets.Button()
    events = []
    w.on_hover.connect(lambda: events.append(True))
    w.on_hover.emit()
    assert events == [True]


def test_layout_linear():
    from sage_engine.gui import manager, widgets, layout

    container = widgets.Button(width=10, height=10)
    child1 = widgets.Button(width=5, height=5)
    child2 = widgets.Button(width=5, height=5)
    container.add_child(child1)
    container.add_child(child2)
    layout.LinearLayout().apply(container)

    assert child1.y == container.y
    assert child2.y == container.y + child1.height


def test_theme():
    from sage_engine.gui import style, widgets

    st = widgets.Button().style
    style.load_theme("retro", "sage_engine/gui/theme/retro.sagegui")
    style.apply_theme(st, "retro")
    assert st.bg_color == (0, 0, 64, 255)

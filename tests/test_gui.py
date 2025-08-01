def test_gui_basic():
    from sage_engine.graphic.manager import GUIManager
    from sage_engine.graphic.widget import Button, Label

    mgr = GUIManager()
    btn = Button(width=10, height=10)
    lbl = Label(width=10, height=10)
    called = []
    btn.on_click.connect(lambda: called.append(True))
    mgr.root.add_child(btn)
    mgr.root.add_child(lbl)

    mgr.dispatch_click(5, 5)
    assert called


from sage_engine.gui import widgets, manager
from sage_engine import gfx


def test_label_button_draw(monkeypatch):
    calls = []
    monkeypatch.setattr(gfx, "draw_text", lambda *a, **k: calls.append(a))
    lbl = widgets.Label(text="hello")
    btn = widgets.Button(text="ok")
    manager.root.add_child(lbl)
    manager.root.add_child(btn)
    manager.draw()
    assert ("hello",) in [ (c[2],) for c in calls ]
    assert ("ok",) in [ (c[2],) for c in calls ]


def test_default_style():
    w = widgets.Label()
    assert w.style.fg_color == (255, 255, 255, 255)
    assert w.style.bg_color == (40, 40, 40, 255)

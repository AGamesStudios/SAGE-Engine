from sage_gui import GUIContext, Label, Button, Checkbox
from sage_gui.core.event import Event, EventType


def test_context_update_and_draw(monkeypatch):
    ctx = GUIContext()
    label = Label(text="hello")
    ctx.add_widget(label)

    called = {}

    def fake_draw(*args, **kwargs):
        called["draw"] = True

    monkeypatch.setattr(label, "draw", fake_draw)
    ctx.update(0.016)
    ctx.draw()
    assert called.get("draw")


def test_button_click(monkeypatch):
    ctx = GUIContext()
    clicked = {}
    btn = Button(text="go", on_click=lambda: clicked.setdefault("ok", True))
    ctx.add_widget(btn)

    evt_move = Event(EventType.MOUSE_MOVE, position=(1, 1))
    evt_down = Event(EventType.MOUSE_DOWN, position=(1, 1))

    btn.width = btn.height = 10
    ctx.dispatch_event(evt_move)
    ctx.dispatch_event(evt_down)
    assert clicked.get("ok")


def test_checkbox_toggle():
    ctx = GUIContext()
    cb = Checkbox()
    ctx.add_widget(cb)
    cb.width = cb.height = 10
    evt_down = Event(EventType.MOUSE_DOWN, position=(1, 1))
    ctx.dispatch_event(evt_down)
    assert cb.checked

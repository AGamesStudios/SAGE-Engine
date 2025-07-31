"""Experimental SAGE Studio using SAGE GUI."""

from sage_gui import GUIContext, Label, Button, VBox
from sage_gui.core.event import Event, EventType


def main() -> None:
    ctx = GUIContext()
    root = VBox()
    root.width = 200
    root.height = 100

    label = Label(text="SAGE Studio")
    label.height = 16

    def _on_click() -> None:
        print("Preview not yet implemented")

    btn = Button(text="Run", on_click=_on_click)
    btn.width = 80
    btn.height = 20

    root.add_child(label)
    root.add_child(btn)
    ctx.add_widget(root)

    # Simple fake update/draw loop
    for _ in range(3):
        ctx.update(0.016)
        ctx.draw()

    # simulate click
    evt_move = Event(EventType.MOUSE_MOVE, position=(10, 30))
    evt_down = Event(EventType.MOUSE_DOWN, position=(10, 30))
    ctx.dispatch_event(evt_move)
    ctx.dispatch_event(evt_down)


if __name__ == "__main__":
    main()

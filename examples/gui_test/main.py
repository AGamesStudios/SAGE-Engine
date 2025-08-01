from sage_engine import window, render, gfx
from sage_engine.graphic.manager import GUIManager
from sage_engine.graphic.widget import Button, Label


def main() -> None:
    window.init("GUI Test", 320, 200)
    render.init(window.get_window_handle())
    gfx.init(320, 200)

    manager = GUIManager()
    manager.debug = True
    label = Label(text="Hello", width=80, height=20)
    button = Button(text="Click", y=30, width=80, height=20)

    def on_click():
        label.text = "Clicked"
    button.on_click.connect(on_click)

    manager.root.add_child(label)
    manager.root.add_child(button)

    running = True
    while running:
        window.poll_events()
        gfx.begin_frame((0, 0, 0, 255))
        manager.draw()
        gfx.flush_frame(window.get_window_handle())
        if window.should_close():
            running = False

    gfx.shutdown()
    render.shutdown()
    window.shutdown()


if __name__ == "__main__":
    main()


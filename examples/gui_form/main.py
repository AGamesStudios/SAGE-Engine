from sage_engine import window, render, gfx
from sage_engine.gui import manager, widgets, layout, style


def main() -> None:
    window.init("GUI Form", 320, 240)
    render.init(window.get_window_handle())
    gfx.init(320, 240)

    style.load_theme("dark", "../../sage_engine/gui/theme/dark.sagegui")
    button = widgets.Button(text="Submit", width=80, height=20)
    input_box = widgets.TextInput(width=100, height=20, y=30)
    layout.LinearLayout().apply(manager.root)

    manager.root.add_child(button)
    manager.root.add_child(input_box)

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

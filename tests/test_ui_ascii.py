from sage_engine.tty import tty_system, screen
from sage_engine.tty.ui_core import UIManager
from sage_engine.tty.box import Box
from sage_engine.tty.list import List


def test_ui_draws_widgets():
    ui = UIManager(theme={"foreground": "white", "accent": "yellow"})
    ui.add(Box(0, 0, 4, 3))
    ui.add(List(1, 1, ["A", "B"]))
    tty_system.clear()
    ui.draw(tty_system)
    output = screen.render(tty_system.buffer)
    lines = output.splitlines()
    assert lines[0].startswith("\x1b[38")  # color code prefix
    assert "A" in lines[1]
    assert "B" in lines[2]

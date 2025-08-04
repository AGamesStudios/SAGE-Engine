import re

from sage_engine.color import Color
from sage_engine.tty import tty_system, draw_text, draw_rect, input as tty_input, screen


def test_draw_text_and_rect():
    tty_system.clear()
    draw_text(0, 0, "ABC", fg="#ff0000")
    draw_rect(0, 1, 3, 1, char="X", fg=Color(0, 255, 0))
    output = screen.render(tty_system.buffer)
    ansi_re = re.compile(r"\x1b\[[0-9;]*m")
    lines = [ansi_re.sub("", line) for line in output.splitlines()]
    assert lines[0].startswith("ABC")
    assert lines[1][:3] == "XXX"


def test_input_mapping():
    tty_input.map_action("confirm", "y")
    tty_input._inject("y")
    tty_input.update()
    assert tty_input.is_pressed("confirm")

"""Minimal TTY demo: draws a box and reacts to input."""

from sage_engine.logger import setup_logging
from sage_engine.tty import tty_system, input as tty_input
from sage_engine.tty.ui_core import ui_manager
from sage_engine.tty.box import Box
from sage_engine.tty.list import List


def main() -> None:
    setup_logging(level="info")
    ui_manager.add(Box(2, 2, 20, 5))
    ui_manager.add(List(3, 3, ["Welcome to SAGE", "Press q to quit"]))
    running = True
    while running:
        tty_system.clear()
        tty_input.update()
        if tty_input.is_pressed("quit"):
            running = False
        ui_manager.draw(tty_system)
        tty_system.flush()


if __name__ == "__main__":
    tty_input.map_action("quit", "q")
    main()

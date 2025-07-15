import os

from sage_engine import gui, ui


def test_button_hover_click():
    os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")
    backend = gui.load_backend("qt6")
    button = ui.Button()
    hover = []
    clicked = []
    button.on_hover.connect(lambda inside: hover.append(inside))
    button.on_click.connect(lambda: clicked.append(True))
    button.hover(True)
    button.click()
    assert hover == [True]
    assert clicked == [True]
    assert backend  # backend created successfully
